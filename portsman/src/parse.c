/*
Copyright (c) 2002, 2003, Anselm R. Garbe
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of the portsman developers nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "includes.h"

/* values for number of pipes in INDEX file */
#define PORT_NAME_VERSION 0
#define PORT_PATH 1
#define PORT_INSTALL_PREFIX 2
#define PORT_DESCR 3
#define PORT_PKGDESCR 4
#define PORT_MAINTAINER 5
#define PORT_CATEGORY 6
#define PORT_BUILD_DEPENDENCY 7
#define PORT_RUN_DEPENDENCY 8
#define PORT_URL 9

/* local global */
TNode *tcat;

/* adds a port pointer to a category */
void
add_port_to_category(Category *cat, Port *p) {
   (cat->num_of_ports)++;
   if ((p->state == STATE_INSTALLED) ||
      (p->state == STATE_INSTALLED_OLDER) ||
      (p->state == STATE_INSTALLED_NEWER))
      (cat->num_of_inst_ports)++;
   add_list_item(cat->lhprts, p);
}

/* creates all categories through the given ports list */
void
create_categories(int num_of_inst_ports) {
   extern Lhd *lhprts;
   extern Lhd *lhcats;
   Iter pitr = lhprts->head;
   Iter citr = lhcats->head;
   Node *n;
   Category *newcat = (Category *)malloc(sizeof(Category));

   while (pitr != NULL) {
      citr = ((Port *)pitr->item)->lhcats->head;
      while (citr != NULL) {
         add_port_to_category((Category *)citr->item, (Port *)pitr->item);
         citr = citr->next;
      }
      pitr = pitr->next;
   }

   /* last but not least we add meta category */
   /* this trick speeds up this parser about four times */
   newcat->type = CATEGORY;
   newcat->name = "All"; 
   newcat->num_of_ports = lhprts->num_of_items;
   newcat->num_of_inst_ports = num_of_inst_ports;
   newcat->num_of_marked_ports = 0;
   newcat->num_of_deinst_ports = 0;
   newcat->lhprts = lhprts;

   /* linking everything together */
   n = (Node *)malloc(sizeof(Node));
   n->item = newcat;
   n->next = lhcats->head;
   lhcats->head = n;
   lhcats->num_of_items++;
}


/* adds a category in sorted order if it does not already exist,
   returns pointer of the new or existing category with name
   "name" */
Category *
add_category(char *name) {
   extern TNode *tcat;
   extern Lhd *lhcats;
   extern void *exists;
   Category *newcat = NULL;

   newcat = (Category *)malloc(sizeof(Category));
   newcat->type = CATEGORY;
   newcat->name = strdup(name); 

   exists = NULL;
   tcat = add_tree_item(tcat, newcat, cmp_name);
   if (exists != NULL) { /* category does exist */
      /* clean up */
      free(newcat->name);
      free(newcat);
      /* use existing pointer for return */
      newcat = (Category *)((TNode *)exists)->item;
   } else { /* category does not exist yet */
      newcat->num_of_ports = 0;
      newcat->num_of_inst_ports = 0;
      newcat->num_of_marked_ports = 0;
      newcat->num_of_inst_ports = 0;
      newcat->num_of_deinst_ports = 0;
      newcat->lhprts = (Lhd *)malloc(sizeof(Lhd));
      newcat->lhprts->num_of_items = 0;
      newcat->lhprts->head = NULL;
   }

   return newcat;
}

/* parses the FreeBSD ports INDEX file and creates a
   list of categories with dedicated ports, it also creates
   a meta-category "all", all lists and categories are
   sorted ascending. */
int
parse_index()
{
   FILE *fd;
   int c, i, readyToken;
   int pipes = 0;
   char tok[MAX_TOKEN];
   extern Config config;
   extern Lhd *lhprts;
   extern Lhd *lhcats;
   extern void *exists;
   extern TNode *tcat;
   TNode *tdirs;
   TNode *tprt = NULL;
   Port *p, *dprt;
   int num_of_inst_ports = 0;

   /* init */
   p = NULL;
   tcat = NULL;

   if ((fd = fopen(config.index_file, "r")) == NULL)
      return ERROR_OPEN_INDEX; /* error */

   /* parse installed pkgs */
   tdirs = parse_dir(config.inst_pkg_dir);

   i = 0;
   readyToken = 0; /* token not ready */
   while (feof(fd) == 0) {
      c = fgetc(fd); /* get next char */
      switch (c) {
         case '|': /* next token */
            readyToken = 1; /* ready token */
            break;
         case '\n': /* end of port */
            readyToken = 1; /* tail token of port ready */
            break;
         default:
            if ((pipes != PORT_CATEGORY) && (pipes != PORT_BUILD_DEPENDENCY)
                  && (pipes != PORT_RUN_DEPENDENCY))
               /* default, no port category, build dep or run dep modus */
               tok[i++] = (char)c;
            break;
      }

      /* to speed up the parsing, we also parse categories here,
         I guessed if it'll be senseful to also parse BDEP and RDEP,
         but this makes no sense until there aren't all entries
         scanned */
      if (pipes == PORT_CATEGORY) { /* port category mode */
         if ((c == ' ') || (c == '|')) {
            if (i > 0) { /* maybe there're ports without a category */
               tok[i] = '\0'; /* terminate current cat token */
               add_list_item(p->lhcats, add_category(tok));
               i = 0; /* reset i */
            }
         } else { /* inside a token */
            tok[i++] = (char)c;
         }
      } else if ((pipes == PORT_BUILD_DEPENDENCY) ||
            (pipes == PORT_RUN_DEPENDENCY)) { /* port build/run dep mode */
         if ((c == ' ') || (c == '|')) {
            if (i > 0) { /* maybe there're ports without a build dep */
               tok[i] = '\0';
               dprt = create_port(tok, tdirs);
               exists = NULL;
               tprt = add_tree_item(tprt, dprt, cmp_name);
               if (exists != NULL) {
                  free_port(dprt);
                  dprt = (Port *)((TNode *)exists)->item;
               } else {
                  if (dprt->state >= STATE_INSTALLED)
                     num_of_inst_ports++;
               }
               if (pipes == PORT_BUILD_DEPENDENCY)
                  add_list_item(p->lhbdep, dprt);
               else if (pipes == PORT_RUN_DEPENDENCY)
                  add_list_item(p->lhrdep, dprt);
               i = 0; /* reset i */
            }
         } else { /* inside a token */
            tok[i++] = (char)c;
         }
      }

      if (readyToken == 1) { /* a new token is ready */
         tok[i] = '\0'; /* terminate current token */
         switch (pipes) {
            case PORT_NAME_VERSION:
               p = create_port(tok, tdirs);
               /* add the port */
               exists = NULL;
               tprt = add_tree_item(tprt, p, cmp_name);
               if (exists != NULL) {
                  free_port(p);
                  p = (Port *)((TNode *)exists)->item;
               } else {
                  if (p->state >= STATE_INSTALLED)
                     num_of_inst_ports++;
               }
               break;
            case PORT_PATH:
               p->path = strdup(tok);
               break;
            case PORT_INSTALL_PREFIX:
               p->instpfx = strdup(tok);
               break;
            case PORT_DESCR:
               p->descr = strdup(tok);
               break;
            case PORT_PKGDESCR:
               p->pathpkgdesc = strdup(tok);
               break;
            case PORT_MAINTAINER:
               p->maintainer = strdup(tok);
               break;
            case PORT_URL:
               p->url = strdup(tok);
               pipes = -1;
               break;
         }
         readyToken = i = 0; /* token processed, not ready token anymore */
         pipes++;
      }
   }
   fclose(fd); /* close INDEX file */

   create_inorder_list(lhcats, tcat);
   create_inorder_list(lhprts, tprt);
   create_categories(num_of_inst_ports);
   free_tree(tdirs);

   /* finished */
   return (0);
}

/* creates linear list of dir names,
   returns list of directories */
TNode *
parse_dir(char *path) {
   struct dirent *dp;
   DIR *dfd;
   TNode *t = NULL;
 
   /* if the directory doesn't exist or there is another reason,
      it returns NULL */
   if ((dfd = opendir(path)) == NULL) return NULL;

   /* generates entries */
   while ((dp = readdir(dfd)) != NULL) {
      /* exlude hidden files or "." and ".." */
      if ((dp->d_name)[0] != '.') {
         t = add_tree_item(t, strdup(dp->d_name), cmp_str);
      }
   }
   closedir(dfd);
   /* finished */

   /* number of entries */
   return t;
}

/* parses tokens, returns list pointer of token list */ 
Lhd *
parse_tokenlist(char *toklist, char *delim) {
   char *tok;
   char *tlist = strdup(toklist);
   Lhd *lh = (Lhd *)malloc(sizeof(Lhd));
   Node *n = NULL;

   /* init */
   lh->head = NULL;
   lh->num_of_items = 0;

   tok = strtok(tlist, delim);
   while (tok != NULL) {
      n = add_list_item_after(lh, n, strdup(tok));
      tok = strtok(NULL, delim);
   }
   free(tlist);

   return lh;
}

/* parses a file and returns a list, where each item is
   a line of the file */
Lhd *
parse_file(char *filepath) {
   FILE *fd;
   char line[MAX_COLS];
   Lhd *lh;
   Line *l;
   Node *n = NULL;

   if ((fd = fopen(filepath, "r")) == NULL)
      return NULL; /* error */

   /* init */
   lh = (Lhd *)malloc(sizeof(Lhd));
   lh->head = NULL;
   lh->num_of_items = 0;

   while (fgets(line, MAX_COLS, fd) != NULL) 
      n = add_list_item_after(lh, n, create_line(line));

   fclose(fd);
 
   return lh;
}

/* parses a plist file and returns a list of all files */
Lhd *
parse_plist(Port *p, char *plistfile) {
   FILE *fd;
   char line[MAX_TOKEN];
   char path[MAX_PATH];
   Lhd *lh;
   Node *n = NULL;

   if ((fd = fopen(plistfile, "r")) == NULL)
      return NULL; /* error */

   /* init */
   lh = (Lhd *)malloc(sizeof(Lhd));
   lh->head = NULL;
   lh->num_of_items = 0;

   while (fgets(line, MAX_TOKEN, fd) != NULL) {
      if ((line[0] != '\0') && (line[0] != ' ')
            && (line[0] != '@') && (line[0] != '%')) {
         /* valid path */
         Plist *pl = (Plist *)malloc(sizeof(Plist));
         line[strlen(line) - 1] = '\0';
         sprintf(path, "%s/%s", p->instpfx, line);
         pl->name = strdup(path);
         pl->exist = (access(path, F_OK) == 0) ? TRUE : FALSE; 
         n = add_list_item_after(lh, n, pl);
      }
   }
   fclose(fd);

   return lh;
}

/* parses compile options of a port through scan for all
   .if defined or .if !defined in port's Makefile */
Lhd *
parse_options(char *mkfile) {
   FILE *fd;
   char line[MAX_TOKEN];
   char option[MAX_PATH];
   char *start;
   char *end;
   Lhd *lh;
   TNode *t = NULL;
   extern void *exists;
   int i;

   if ((fd = fopen(mkfile, "r")) == NULL)
      return NULL; /* error */

   /* init */
   lh = (Lhd *)malloc(sizeof(Lhd));
   lh->head = NULL;
   lh->num_of_items = 0;

   while (fgets(line, MAX_TOKEN, fd) != NULL) {
      if ((strstr(line, ".if") != NULL) &&
            (strstr(line, "defined") != NULL)) {
         Option *opt = (Option *)malloc(sizeof(Option));
         opt->type = OPTION;
         opt->state = STATE_NOT_SELECTED;
         i = 0;
         start = strpbrk(line, "({");
         start++;
         end = strpbrk(line, ")}");
         while (start != end)
            option[i++] = *start++;
         option[i] = '\0';
         sprintf(line, "%-.50s (port specific compile option)", option);
         opt->name = strdup(line);
         sprintf(line, "%s=yes", option);
         opt->arg = strdup(line);
         exists = NULL;
         t = add_tree_item(t, opt, cmp_name);
         if (exists != NULL) { /* option does exist */
            /* clean up */
            free(opt->name);
            free(opt->arg);
            free(opt);
         }
      }
   }
   fclose(fd);

   create_inorder_list(lh, t);
   return lh;
}

/* returns the color of an associated string,
   -1 if color could not be indicated,
   this is a helper function for parse_rc_file */
short
str_to_color(char *clrstr) {
   if (strcmp(clrstr, "BLACK") == 0)
      return COLOR_BLACK;
   else if (strcmp(clrstr, "RED") == 0)
      return COLOR_RED;
   else if (strcmp(clrstr, "GREEN") == 0)
      return COLOR_GREEN;
   else if (strcmp(clrstr, "YELLOW") == 0)
      return COLOR_YELLOW;
   else if (strcmp(clrstr, "BLUE") == 0)
      return COLOR_BLUE;
   else if (strcmp(clrstr, "MAGENTA") == 0)
      return COLOR_MAGENTA;
   else if (strcmp(clrstr, "CYAN") == 0)
      return COLOR_CYAN;
   else if (strcmp(clrstr, "WHITE") == 0)
      return COLOR_WHITE;
   else
      return ERROR_CORRUPT_RC_FILE; /* error */
}

short
str_to_state(char *boolstr) {
   if (strcmp(boolstr, "TRUE") == 0)
      return STATE_SELECTED;
   else if (strcmp(boolstr, "FALSE") == 0)
      return STATE_NOT_SELECTED;
   else
      return ERROR_CORRUPT_RC_FILE;
}

/* parses portsmanrc file and resets config if needed,
   returns 0 if everything was ok, ERROR_NO_RC_FILE if
   file could not be opened or a positive int of the
   line number where ERROR_CORRUPT_RC_FILE succeeded or
   an unknown keyword exist */
int
parse_rc_file(char *filepath) {
   FILE *fd;
   extern Config config;
   bool readyKey = FALSE;
   bool readyValue = FALSE;
   bool comment = FALSE;
   char tok[MAX_TOKEN];
   char arg[MAX_TOKEN];
   char *key;
   char *val;
   int i = 0, c = 0, line = 0;
   short sh = 0;
   
   if ((fd = fopen(filepath, "r")) == NULL)
      return ERROR_NO_RC_FILE; /* not present */

   while (feof(fd) == 0) {
      c = fgetc(fd); /* get next char */
      switch (c) {
         case '\t': 
         case ' ': /* ignore all white spaces & tabs*/
            break;
         case '#':
            comment = TRUE; /* until eol */
            if (i > 0) /* ready value */
               readyValue = TRUE;
            break;
         case '\n': /* eol */
            line++;
            comment = FALSE;
            /* ready value */
            if (i > 0)  /* ready value */
               readyValue = TRUE;
            break;
         case '=':
            readyKey = TRUE;
            break;
         default: /* else it's a alphanum char */
            if (!comment) 
               tok[i++] = (char)c;
            break;
      }

      if (readyKey) {
         tok[i] = '\0'; /* terminate key token */
         key = strdup(tok);
         i = 0;
         readyKey = FALSE;
      } else if (readyValue) {
         tok[i] = '\0'; /* terminate value token */
         val = tok;

         if (strcmp(key, "titlebar.fcolor") == 0) {
            sh = str_to_color(val);
            if (sh == ERROR_CORRUPT_RC_FILE) {
               return (line); /* error */
            } else {
               config.fcolors[CLR_TITLE] = sh;
            }
         } else if (strcmp(key, "titlebar.bcolor") == 0) {
            sh = str_to_color(val);
            if (sh == ERROR_CORRUPT_RC_FILE) {
               return (line); /* error */
            } else {
               config.bcolors[CLR_TITLE] = sh;
            }
         } else if (strcmp(key, "browser.fcolor") == 0) {
            sh = str_to_color(val);
            if (sh == ERROR_CORRUPT_RC_FILE) {
               return (line); /* error */
            } else {
               config.fcolors[CLR_BROWSE] = sh;
            }
         } else if (strcmp(key, "browser.bcolor") == 0) {
            sh = str_to_color(val);
            if (sh == ERROR_CORRUPT_RC_FILE) {
               return (line); /* error */
            } else {
               config.bcolors[CLR_BROWSE] = sh;
            }
         } else if (strcmp(key, "statusbar.fcolor") == 0) {
            sh = str_to_color(val);
            if (sh == ERROR_CORRUPT_RC_FILE) {
               return (line); /* error */
            } else {
               config.fcolors[CLR_STATUS] = sh;
            }
         } else if (strcmp(key, "statusbar.bcolor") == 0) {
            sh = str_to_color(val);
            if (sh == ERROR_CORRUPT_RC_FILE) {
               return (line); /* error */
            } else {
               config.bcolors[CLR_STATUS] = sh;
            }
         } else if (strcmp(key, "cmdbar.fcolor") == 0) {
            sh = str_to_color(val);
            if (sh == ERROR_CORRUPT_RC_FILE) {
               return (line); /* error */
            } else {
               config.fcolors[CLR_CMD] = sh;
            }
         } else if (strcmp(key, "cmdbar.bcolor") == 0) {
            sh = str_to_color(val);
            if (sh == ERROR_CORRUPT_RC_FILE) {
               return (line); /* error */
            } else {
               config.bcolors[CLR_CMD] = sh;
            }
         } else if (strcmp(key, "selector.fcolor") == 0) {
            sh = str_to_color(val);
            if (sh == ERROR_CORRUPT_RC_FILE) {
               return (line); /* error */
            } else {
               config.fcolors[CLR_SELECTOR] = sh;
            }
         } else if (strcmp(key, "selector.bcolor") == 0) {
            sh = str_to_color(val);
            if (sh == ERROR_CORRUPT_RC_FILE) {
               return (line); /* error */
            } else {
               config.bcolors[CLR_SELECTOR] = sh;
            }
         } else if (strcmp(key, "indexfile") == 0) {
            config.index_file = strdup(val);
         } else if (strcmp(key, "pkgdir") == 0) {
            config.inst_pkg_dir = strdup(val);
         } else if (strcmp(key, "make.cmd") == 0) {
            config.make_cmd = strdup(val);
         } else if (strcmp(key, "make.target.inst") == 0) {
            config.make_target[MK_TARGET_INST] = strdup(val);
         } else if (strcmp(key, "make.target.deinst") == 0) {
            config.make_target[MK_TARGET_DEINST] = strdup(val);
         } else if (strcmp(key, "make.target.update") == 0) {
            config.make_target[MK_TARGET_UPDATE] = strdup(val);
         } else if (strcmp(key, "make.option.force") == 0) {
            sh = str_to_state(val);
            if (sh == ERROR_CORRUPT_RC_FILE) {
               return (line); /* error */
            } else {
               config.make_option[MK_OPTION_FORCE] = sh;
            }
         } else if (strcmp(key, "make.option.pkg") == 0) {
            sh = str_to_state(val);
            if (sh == ERROR_CORRUPT_RC_FILE) {
               return (line); /* error */
            } else {
               config.make_option[MK_OPTION_PKG] = sh;
            }
         } else if (strcmp(key, "make.option.clean") == 0) {
            sh = str_to_state(val);
            if (sh == ERROR_CORRUPT_RC_FILE) {
               return (line); /* error */
            } else {
               config.make_option[MK_OPTION_CLEAN] = sh;
            }
         } else if (strcmp(key, "make.option.nochksum") == 0) {
            sh = str_to_state(val);
            if (sh == ERROR_CORRUPT_RC_FILE) {
               return (line); /* error */
            } else {
               config.make_option[MK_OPTION_NOCHKSUM] = sh;
            }
         } else if (strcmp(key, "make.option.nodeps") == 0) {
            sh = str_to_state(val);
            if (sh == ERROR_CORRUPT_RC_FILE) {
               return (line); /* error */
            } else {
               config.make_option[MK_OPTION_NODEPS] = sh;
            }
         } else if (strcmp(key, "make.option.forcepkgreg") == 0) {
            sh = str_to_state(val);
            if (sh == ERROR_CORRUPT_RC_FILE) {
               return (line); /* error */
            } else {
               config.make_option[MK_OPTION_FORCEPKGREG] = sh;
            }
         } else if (strcmp(key, "make.option.nopkgreg") == 0) {
            sh = str_to_state(val);
            if (sh == ERROR_CORRUPT_RC_FILE) {
               return (line); /* error */
            } else {
               config.make_option[MK_OPTION_NOPKGREG] = sh;
            }
         } else if (strcmp(key, "make.option.force.arg") == 0) {
            sprintf(arg, "%s=yes", val);
            config.make_option_arg[MK_OPTION_FORCE] = strdup(arg);
         } else if (strcmp(key, "make.option.pkg.arg") == 0) {
            sprintf(arg, "%s=yes", val);
            config.make_option_arg[MK_OPTION_PKG] = strdup(arg);
         } else if (strcmp(key, "make.option.clean.arg") == 0) {
            sprintf(arg, "%s=yes", val);
            config.make_option_arg[MK_OPTION_CLEAN] = strdup(arg);
         } else if (strcmp(key, "make.option.nochksum.arg") == 0) {
            sprintf(arg, "%s=yes", val);
            config.make_option_arg[MK_OPTION_NOCHKSUM] = strdup(arg);
         } else if (strcmp(key, "make.option.nodeps.arg") == 0) {
            sprintf(arg, "%s=yes", val);
            config.make_option_arg[MK_OPTION_NODEPS] = strdup(arg);
         } else if (strcmp(key, "make.option.forcepkgreg.arg") == 0) {
            sprintf(arg, "%s=yes", val);
            config.make_option_arg[MK_OPTION_FORCEPKGREG] = strdup(arg);
         } else if (strcmp(key, "make.option.nopkgreg.arg") == 0) {
            sprintf(arg, "%s=yes", val);
            config.make_option_arg[MK_OPTION_NOPKGREG] = strdup(arg);
         } else {
            return line; /* also error: unknown key */
         }
            
         free(key);
         i = 0;
         readyValue = FALSE;
      }

   }
   fclose(fd);

   return (0);
}

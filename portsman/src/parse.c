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
TNode *trootcat;

/* adds a port pointer to a category */
void
add_port_to_category(Category *cat, Port *port) {
   (cat->num_of_ports)++;
   if ((port->state == STATE_INSTALLED) ||
      (port->state == STATE_INSTALLED_OLDER) ||
      (port->state == STATE_INSTALLED_NEWER))
      (cat->num_of_inst_ports)++;
   add_list_item(cat->lports, port);
}

/* creates all categories through the given ports list */
void
create_categories() {
   extern State state;
   extern List *lports;
   extern List *lcats;
   Node *np = lports->head;
   Node *nc = lcats->head;
   Category *newcat = (Category *)malloc(sizeof(Category));

   while (np != NULL) {
      nc = ((Port *)np->item)->lcats->head;
      while (nc != NULL) {
         add_port_to_category((Category *)nc->item, (Port *)np->item);
         nc = nc->next;
      }
      np = np->next;
   }

   /* last but not least we add meta category */
   /* this trick speeds up this parser about four times */
   newcat->type = CATEGORY;
   newcat->name = "All"; 
   newcat->num_of_ports = state.num_of_ports;
   newcat->num_of_inst_ports = state.num_of_inst_ports;
   newcat->num_of_marked_ports = state.num_of_marked_ports;
   newcat->num_of_deinst_ports = state.num_of_deinst_ports;
   newcat->lports = lports;

   /* linking everything together */
   np = (Node *)malloc(sizeof(Node));
   np->item = newcat;
   np->next = lcats->head;
   lcats->head = np;
   lcats->num_of_items++;
}


/* adds a category in sorted order if it does not already exist,
   returns pointer of the new or existing category with name
   "name" */
Category *
add_category(char *name) {
   extern TNode *trootcat;
   extern List *lcats;
   extern void *exists;
   Category *newcat = NULL;

   newcat = (Category *)malloc(sizeof(Category));
   newcat->type = CATEGORY;
   newcat->name = strdup(name); 

   exists = NULL;
   trootcat = add_tree_item(trootcat, newcat, cmp_name);
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
      newcat->lports = (List *)malloc(sizeof(List));
      newcat->lports->num_of_items = 0;
      newcat->lports->head = NULL;
   }

   return newcat;
}

/* returns state of the port with portname */
short
get_state(char *portname, TNode *trootdirs) {
   TNode *t = trootdirs;
   char *p, *pv, *d, *dv;
   int cmp;

   /* first of all, check, if this port is equal to an installed port */
   while (t != NULL) {
      cmp = strcmp((char *)t->item, portname);
      if (cmp == 0)
         return STATE_INSTALLED; /* found equal */
      else if (cmp > 0) /* use left child */
         t = t->left;
      else
         t = t->right;
   }
 
   /* now we're sure, that this port is not installed or newer/older than
      the installed version, so check this */
   t = trootdirs;
   while (t != NULL) {
      p = portname;
      d = (char *)t->item;
      pv = strchr(portname, '.'); 
      if (pv == NULL)
         pv = strrchr(portname, '-');
      dv = strchr(d, '.'); 
      if (dv == NULL)
         dv = strrchr(d, '-');

      for(; *p == *d; p++, d++)
         if ((p == pv) || (d == dv)) /* like strcmp of first chars until pv/dv */
            break;

      if ((p == pv) && (d == dv)) { /* seems to be older or newer */
         if (strcmp(pv, dv) > 0)
            return STATE_INSTALLED_NEWER;
         else
            return STATE_INSTALLED_OLDER;

      } else {

         if (strcmp((char *)t->item, portname) > 0) /* use left child */
            t = t->left;
         else
            t = t->right;
      }
   }

   return STATE_NOT_SELECTED;
}

/* frees a port */
void
free_port(Port *port) {

   free_list(port->lcats);
   if (port->lopts != NULL)
      free_list(port->lopts);
   free_list(port->lbdep);
   free_list(port->lrdep);
   free(port->name);
   free(port);
}

/* returns a new allocated port with all standard
   initialization */
Port *
create_port(char *name, TNode *trootdirs) {

   /* alloc mem for new port */
   Port *port = (Port *)malloc(sizeof(Port));

   /* init */
   port->type = PORT;
   port->lcats = (List *)malloc(sizeof(List));
   port->lcats->head = NULL;
   port->lcats->num_of_items = 0;
   port->lopts = NULL;
   port->lbdep = (List *)malloc(sizeof(List));
   port->lbdep->head = NULL;
   port->lbdep->num_of_items = 0;
   port->lrdep = (List *)malloc(sizeof(List));
   port->lrdep->head = NULL;
   port->lrdep->num_of_items = 0;
   port->name = strdup(name);
   port->state = get_state(port->name, trootdirs);

   return port;
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
   extern State state;
   extern Config config;
   extern List *lports;
   extern List *lcats;
   extern void *exists;
   extern TNode *trootcat;
   TNode *trootdirs;
   TNode *trootport = NULL;
   Port *port, *depport;

   /* init */
   port = NULL;
   trootcat = NULL;

   if ((fd = fopen(config.index_file, "r")) == NULL)
      return ERROR_OPEN_INDEX; /* error */

   /* parse installed pkgs */
   trootdirs = parse_dir(config.inst_pkg_dir);
   if (trootdirs == NULL) /* error */
      return ERROR_OPEN_PDB_DIR;

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
               add_list_item(port->lcats, add_category(tok));
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
               depport = create_port(tok, trootdirs);
               exists = NULL;
               trootport = add_tree_item(trootport, depport, cmp_name);
               if (exists != NULL) {
                  free_port(depport);
                  depport = (Port *)((TNode *)exists)->item;
               } else {
                  if (depport->state >= STATE_INSTALLED)
                     state.num_of_inst_ports++;
               }
               if (pipes == PORT_BUILD_DEPENDENCY)
                  add_list_item(port->lbdep, depport);
               else if (pipes == PORT_RUN_DEPENDENCY)
                  add_list_item(port->lrdep, depport);
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
               port = create_port(tok, trootdirs);
               /* add the port */
               exists = NULL;
               trootport = add_tree_item(trootport, port, cmp_name);
               if (exists != NULL) {
                  free_port(port);
                  port = (Port *)((TNode *)exists)->item;
               } else {
                  if (port->state >= STATE_INSTALLED)
                     state.num_of_inst_ports++;
               }
               break;
            case PORT_PATH:
               port->path = strdup(tok);
               break;
            case PORT_INSTALL_PREFIX:
               port->instpfx = strdup(tok);
               break;
            case PORT_DESCR:
               port->descr = strdup(tok);
               break;
            case PORT_PKGDESCR:
               port->pathpkgdesc = strdup(tok);
               break;
            case PORT_MAINTAINER:
               port->maintainer = strdup(tok);
               break;
            case PORT_URL:
               port->url = strdup(tok);
               state.num_of_ports++;
               pipes = -1;
               break;
         }
         readyToken = i = 0; /* token processed, not ready token anymore */
         pipes++;
      }
   }
   fclose(fd); /* close INDEX file */

   /* important to set state before, because create_categories
      needs it */
   create_inorder_list(lcats, trootcat);
   create_inorder_list(lports, trootport);
   create_categories();
   free_tree(trootdirs);

   state.num_of_cats = lcats->num_of_items;
   /* finished */
   return (0);
}

/* creates linear list of dir names,
   returns list of directories */
TNode *
parse_dir(char *path) {
   struct dirent *dp;
   DIR *dfd;
   TNode *troot = NULL;
 
   /* if the directory doesn't exist or there is another reason,
      it returns NULL */
   if ((dfd = opendir(path)) == NULL) return NULL;

   /* generates entries */
   while ((dp = readdir(dfd)) != NULL) {
      /* exlude hidden files or "." and ".." */
      if ((dp->d_name)[0] != '.') {
         troot = add_tree_item(troot, strdup(dp->d_name), cmp_str);
      }
   }
   closedir(dfd);
   /* finished */

   /* number of entries */
   return troot;
}

/* parses tokens, returns list pointer of token list */ 
List *
parse_tokenlist(char *toklist, char *delim) {
   char *tok;
   char *tlist = strdup(toklist);
   List *l = (List *)malloc(sizeof(List));
   Node *prevtok = NULL;

   /* init */
   l->head = NULL;
   l->num_of_items = 0;

   tok = strtok(tlist, delim);
   while (tok != NULL) {
      prevtok = add_list_item_after(l, prevtok, strdup(tok));
      tok = strtok(NULL, delim);
   }
   free(tlist);

   return l;
}

/* parses a plist file and returns a list of all files */
List *
parse_plist(Port *port, char *plistfile) {
   FILE *fd;
   char line[MAX_TOKEN];
   char path[MAX_PATH];
   List *l = (List *)malloc(sizeof(List));
   Node *n = NULL;

   if ((fd = fopen(plistfile, "r")) == NULL)
      return NULL; /* error */

   /* init */
   l->head = NULL;
   l->num_of_items = 0;

   while (fgets(line, MAX_TOKEN, fd) != NULL) {
      if ((line[0] != '\0') && (line[0] != ' ')
            && (line[0] != '@') && (line[0] != '%')) {
         /* valid path */
         Plist *pl = (Plist *)malloc(sizeof(Plist));
         line[strlen(line) - 1] = '\0';
         sprintf(path, "%s/%s", port->instpfx, line);
         pl->name = strdup(path);
         pl->exist = (access(path, F_OK) == 0) ? TRUE : FALSE; 
         n = add_list_item_after(l, n, pl);
      }
   }
   fclose(fd);

   return l;
}

/* parses compile options of a port through scan for all
   .if defined or .if !defined in port's Makefile */
List *
parse_options(char *mkfile) {
   FILE *fd;
   char line[MAX_TOKEN];
   char option[MAX_PATH];
   char *start;
   char *end;
   List *l = (List *)malloc(sizeof(List));
   TNode *troot = NULL;
   extern void *exists;
   int i;

   if ((fd = fopen(mkfile, "r")) == NULL)
      return NULL; /* error */

   /* init */
   l->head = NULL;
   l->num_of_items = 0;

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
         opt->cmd = strdup(line);
         exists = NULL;
         troot = add_tree_item(troot, opt, cmp_name);
         if (exists != NULL) { /* option does exist */
            /* clean up */
            free(opt->name);
            free(opt->cmd);
            free(opt);
         }
      }
   }
   fclose(fd);

   create_inorder_list(l, troot);
   return l;
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

/* parses portsmanrc file and resets config if needed */
int
parse_rc_file(char *filepath) {
   FILE *fd;
   extern Config config;
   bool readyKey = FALSE;
   bool readyValue = FALSE;
   bool comment = FALSE;
   char tok[MAX_TOKEN];
   char *key;
   char *val;
   int i = 0, c = 0;
   short clr = 0;
   
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
            clr = str_to_color(val);
            if (clr == ERROR_CORRUPT_RC_FILE) {
               return (ERROR_CORRUPT_RC_FILE); /* error */
            } else {
               config.fcolors[CLR_TITLE] = clr;
            }
         } else if (strcmp(key, "titlebar.bcolor") == 0) {
            clr = str_to_color(val);
            if (clr == ERROR_CORRUPT_RC_FILE) {
               return (ERROR_CORRUPT_RC_FILE); /* error */
            } else {
               config.bcolors[CLR_TITLE] = clr;
            }
         } else if (strcmp(key, "browser.fcolor") == 0) {
            clr = str_to_color(val);
            if (clr == ERROR_CORRUPT_RC_FILE) {
               return (ERROR_CORRUPT_RC_FILE); /* error */
            } else {
               config.fcolors[CLR_BROWSE] = clr;
            }
         } else if (strcmp(key, "browser.bcolor") == 0) {
            clr = str_to_color(val);
            if (clr == ERROR_CORRUPT_RC_FILE) {
               return (ERROR_CORRUPT_RC_FILE); /* error */
            } else {
               config.bcolors[CLR_BROWSE] = clr;
            }
         } else if (strcmp(key, "statusbar.fcolor") == 0) {
            clr = str_to_color(val);
            if (clr == ERROR_CORRUPT_RC_FILE) {
               return (ERROR_CORRUPT_RC_FILE); /* error */
            } else {
               config.fcolors[CLR_STATUS] = clr;
            }
         } else if (strcmp(key, "statusbar.bcolor") == 0) {
            clr = str_to_color(val);
            if (clr == ERROR_CORRUPT_RC_FILE) {
               return (ERROR_CORRUPT_RC_FILE); /* error */
            } else {
               config.bcolors[CLR_STATUS] = clr;
            }
         } else if (strcmp(key, "cmdbar.fcolor") == 0) {
            clr = str_to_color(val);
            if (clr == ERROR_CORRUPT_RC_FILE) {
               return (ERROR_CORRUPT_RC_FILE); /* error */
            } else {
               config.fcolors[CLR_CMD] = clr;
            }
         } else if (strcmp(key, "cmdbar.bcolor") == 0) {
            clr = str_to_color(val);
            if (clr == ERROR_CORRUPT_RC_FILE) {
               return (ERROR_CORRUPT_RC_FILE); /* error */
            } else {
               config.bcolors[CLR_CMD] = clr;
            }
         } else if (strcmp(key, "selector.fcolor") == 0) {
            clr = str_to_color(val);
            if (clr == ERROR_CORRUPT_RC_FILE) {
               return (ERROR_CORRUPT_RC_FILE); /* error */
            } else {
               config.fcolors[CLR_SELECTOR] = clr;
            }
         } else if (strcmp(key, "selector.bcolor") == 0) {
            clr = str_to_color(val);
            if (clr == ERROR_CORRUPT_RC_FILE) {
               return (ERROR_CORRUPT_RC_FILE); /* error */
            } else {
               config.bcolors[CLR_SELECTOR] = clr;
            }
         } else if (strcmp(key, "indexfile") == 0) {
            config.index_file = strdup(val);
         } else if (strcmp(key, "pkgdir") == 0) {
            config.inst_pkg_dir = strdup(val);
         }

         free(key);
         i = 0;
         readyValue = FALSE;
      }

   }
   fclose(fd);

   return (0);
}

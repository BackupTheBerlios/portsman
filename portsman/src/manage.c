/*
   Copyright (c) 2002, 2003, Anselm R. Garbe
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 * Neither the name of the portsman developers nor the names of its
 contributors may be used to endorse or promote products derived from this
 software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */
#include "includes.h"
#include "res.h" /* help file */

/* returns list of online help */
List *
get_online_help() {
   List *l;
   int i;

   /* init */
   l = (List *)malloc(sizeof(List));
   l->head = NULL;
   l->tail = NULL;
   l->num_of_items = 0;

   for (i = 0; i < (sizeof(online_help)/sizeof(online_help[0])); i++)
      add_list_item(l, create_line((char *)online_help[i]));

   return l;
}

/* frees a port */
void
free_port(Port *p) {

   free_list(p->lcats);
   if (p->lopts != NULL)
      free_list(p->lopts);
   free_list(p->lbdep);
   free_list(p->lrdep);
   free(p->name);
   free(p);
}

/* returns state of the port with portname, t has to be
   root of tree with dir names (char *) */
short
get_state(char *portname, TNode *t) {
   TIter titr = t;
   char *p, *pv, *d, *dv;
   int cmp;

   /* first of all, check, if this port is equal to an installed port */
   while (titr != NULL) {
      cmp = strcmp((char *)titr->item, portname);
      if (cmp == 0)
         return STATE_INSTALLED; /* found equal */
      else if (cmp > 0) /* use left child */
         titr = titr->left;
      else
         titr = titr->right;
   }

   /* now we're sure, that this port is not installed or newer/older than
      the installed version, so check this */
   titr = t;
   while (titr != NULL) {
      p = portname;
      d = (char *)titr->item;
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

         if (strcmp((char *)titr->item, portname) > 0) /* use left child */
            titr = titr->left;
         else
            titr = titr->right;
      }
   }

   return STATE_NOT_SELECTED;
}


/* returns a new allocated port with all standard
   initialization */
Port *
create_port(char *name, TNode *t) {

   /* alloc mem for new port */
   Port *p = (Port *)malloc(sizeof(Port));

   /* init */
   p->type = PORT;
   p->lcats = (List *)malloc(sizeof(List));
   p->lcats->head = NULL;
   p->lcats->tail = NULL;
   p->lcats->num_of_items = 0;
   p->lopts = NULL;
   p->lbdep = (List *)malloc(sizeof(List));
   p->lbdep->head = NULL;
   p->lbdep->tail = NULL;
   p->lbdep->num_of_items = 0;
   p->lrdep = (List *)malloc(sizeof(List));
   p->lrdep->head = NULL;
   p->lrdep->tail = NULL;
   p->lrdep->num_of_items = 0;
   p->ldep = (List *)malloc(sizeof(List));
   p->ldep->head = NULL;
   p->ldep->tail = NULL;
   p->ldep->num_of_items = 0;
   p->name = strdup(name);
   p->state = get_state(p->name, t);

   return p;
}

Line *
create_line(char *name) {
   Line *l = (Line *)malloc(sizeof(Line));
   l->type = LINE;
   l->name = strdup(name);
   return l;
}

void
free_line(Line *l) {
   free(l->name);
   free(l);
}

/* marks all ports of list of l with specific state, if possible */
void
mark_ports(List *l, int state) {
   Iter itr = l->head;
   Port *p;

   while (itr != NULL) {
      p = (Port *)itr->item;
      switch (state) {
         case STATE_INSTALL:
            if (p->state == STATE_NOT_SELECTED) {
               mark_port(p, STATE_INSTALL, 1);
               mark_dependencies(p);
            }
            break;
         case STATE_UPDATE:
            if ((p->state == STATE_INSTALLED) ||
                  (p->state == STATE_INSTALLED_OLDER)) {
               mark_port(p, STATE_UPDATE, 1);
               mark_dependencies(p);
            }
            break;
         case STATE_DEINSTALL:
            if (p->state >= STATE_INSTALLED) {
               mark_port(p, STATE_DEINSTALL, 1);
            }
         case STATE_NOT_SELECTED:
            if ((p->state == STATE_INSTALL) ||
                  (p->state == STATE_UPDATE)) {
               mark_port(p, STATE_NOT_SELECTED, -1);
            }
            break;   
      }
      itr = itr->next;
   }

   /* to speed up this algorithm */
   if ((state == STATE_DEINSTALL) || (state == STATE_NOT_SELECTED))
      unmark_all_dependencies();

}

/* creates fake category for filter browsing, type means state or
   string, item contains state or search string */
Category *
create_filter_category(List *lfilter, char *name,
      int type, void *item) {
   List *l = (List *)malloc(sizeof(List));
   Category *cat = (Category *)malloc(sizeof(Category));
   Iter itr = lfilter->head;
   Port *p;

   /* init */
   l->head = NULL;
   l->tail = NULL;
   l->num_of_items = 0;
   cat->num_of_ports = 0;
   cat->num_of_marked_ports = 0;
   cat->num_of_inst_ports = 0;
   cat->num_of_deinst_ports = 0;

   if (type == STATE) { /* filter ports through state */
      int state = *(int *)item;
      while (itr != NULL) {
         p = (Port *)itr->item;
         if (p->state == state) {
            add_list_item(l, p);

            switch (p->state) {
               case STATE_INSTALL:
               case STATE_UPDATE:
                  (cat->num_of_marked_ports)++;
                  break;
               case STATE_DEINSTALL:
                  (cat->num_of_deinst_ports)++;
                  break;
               case STATE_INSTALLED:
               case STATE_INSTALLED_NEWER:
               case STATE_INSTALLED_OLDER:
               case STATE_BDEP:
               case STATE_RDEP:
                  (cat->num_of_inst_ports)++;
                  break;
            }
         } 
         itr = itr->next;
      } 
   } else if (type == STRING) { /* filter ports through search string */
      char *s = (char *)item;
      while (itr != NULL) {
         p = (Port *)itr->item;
         if ((str_str(p->name, s) != NULL) ||
               (str_str(p->descr, s) != NULL)) {
            add_list_item(l, p);

            switch (p->state) {
               case STATE_INSTALL:
               case STATE_UPDATE:
                  (cat->num_of_marked_ports)++;
                  break;
               case STATE_DEINSTALL:
                  (cat->num_of_deinst_ports)++;
                  break;
               case STATE_INSTALLED:
               case STATE_INSTALLED_NEWER:
               case STATE_INSTALLED_OLDER:
                  (cat->num_of_inst_ports)++;
                  break;
            }
         }
         itr = itr->next;
      }
   } else if (type == DEPENDENCY) {
      /* list of all installed ports is always second category of linst */
      Iter ditr;
      int inst;

      while (itr != NULL) {
         p = (Port *)itr->item;
         if (p->state >= STATE_INSTALLED) {
            ditr = p->ldep->head;
            inst = 0;
            if (ditr != NULL) { /* don't care about standalone ports */
               while (ditr != NULL) {
                  if (((Port *)ditr->item)->state >= STATE_INSTALLED) {
                     inst++;
                     break; /* not a port we're seeking for, there are
                               installed dependencies */
                  }
                  ditr = ditr->next;
               }   
               if (inst == 0) {
                  /* found a relevant port, which is installed,
                     and which is a dependency for no installed
                     ports, maybe it is redundant, so filter */
                  add_list_item(l, p);

                  /* p must be installed */
                  (cat->num_of_inst_ports)++;
               }
            }
         }
         itr = itr->next;
      }
   } else if (type == REMOVAL) {
      List *lplist;
      Iter pitr;
      char plistfile[MAX_PATH];
      bool brk = FALSE;

      while (itr != NULL) {
         p = (Port *)itr->item;
         if (p->state < STATE_INSTALLED) {
            sprintf(plistfile, "%s/pkg-plist", p->path);
            if ((lplist = parse_plist(p, plistfile)) != NULL) {
               pitr = lplist->head;
               while (pitr != NULL) {
                  Plist *pl = (Plist *)pitr->item;
                  if (pl->exist == TRUE) {
                     add_list_item(l, p);
                     switch (p->state) {
                        case STATE_INSTALL:
                        case STATE_UPDATE:
                           (cat->num_of_marked_ports)++;
                           break;
                        case STATE_DEINSTALL:
                           (cat->num_of_deinst_ports)++;
                           break;
                        case STATE_INSTALLED:
                        case STATE_INSTALLED_NEWER:
                        case STATE_INSTALLED_OLDER:
                           (cat->num_of_inst_ports)++;
                           break;
                     }
                     brk = TRUE; /* found, so break */
                  }
                  /* clean up */
                  free(pl->name);
                  free(pl);
                  if (brk == TRUE) {
                     brk = FALSE;
                     break; /* break now, after cleaning up */
                  }
                  pitr = pitr->next;
               }
               free_list(lplist);
            }
         }
         itr = itr->next;
      }
   }


   /* creates a fake category to rape browse_list for filter browser
      features */
   cat->type = CATEGORY;
   cat->name = strdup(name); 
   cat->meta = TRUE;
   cat->num_of_ports = l->num_of_items;
   cat->lprts = l;

   return cat;
}

/* creates fake category for (de)install/update proceed browsing */
Category *
create_proceed_category() {

   extern List *lprts;
   List *l = (List *)malloc(sizeof(List));
   Category *cat = (Category *)malloc(sizeof(Category));
   Iter pitr = lprts->head;
   Iter oitr = NULL;
   Node *n = NULL;
   Port *p;
   int num_of_deinst_ports = 0;
   int num_of_marked_ports = 0;

   l->head = NULL;
   l->tail = NULL;
   l->num_of_items = 0;
   while (pitr != NULL) {
      p = (Port *)pitr->item;
      if ((p->state == STATE_INSTALL) ||
            (p->state == STATE_UPDATE) || 
            (p->state == STATE_DEINSTALL))  {
         add_list_item(l, p);
         if (p->state != STATE_DEINSTALL) {
            if (p->lopts == NULL) create_options(p);
            oitr = p->lopts->head;
            /* add also all options to the list */
            while (oitr != NULL) {
               add_list_item(l, (Option *)oitr->item);
               oitr = oitr->next;
            }
            num_of_marked_ports++;
         } else {
            num_of_deinst_ports++;
         }
      }   
      pitr = pitr->next;
   }

   /* creates a fake category to rape browse_list for proceed browser
      features */
   cat->type = CATEGORY;
   cat->name = "(de)installation/upgrade"; 
   cat->meta = TRUE;
   cat->num_of_ports = l->num_of_items;
   cat->num_of_inst_ports = 0;
   cat->num_of_marked_ports = num_of_marked_ports;
   cat->num_of_deinst_ports = num_of_deinst_ports;
   cat->lprts = l;

   return cat;
}

/* creates all possible options to this port */
void
create_options(Port *p) {
   extern Config config;
   List *l = (List *)malloc(sizeof(List));
   List *lopts = NULL;
   Option *opt;
   Iter itr;
   char mkfile[MAX_PATH];

   /* init */
   l->head = NULL;
   l->tail = NULL;
   l->num_of_items = 0;
   sprintf(mkfile, "%s/Makefile", p->path);
   lopts = parse_options(mkfile);

   /* first of all, add standard option and make targets */
   opt = (Option *)malloc(sizeof(Option));
   opt->type = OPTION;
   opt->name = "ignore all errors while compilation of port";
   opt->arg = config.make_option_arg[MK_OPTION_FORCE];
   opt->state = config.make_option[MK_OPTION_FORCE];
   add_list_item(l, opt);

   opt = (Option *)malloc(sizeof(Option));
   opt->type = OPTION;
   opt->name = "generate a package of the port after install/update";
   opt->arg = config.make_option_arg[MK_OPTION_PKG];
   opt->state = config.make_option[MK_OPTION_PKG];
   add_list_item(l, opt);

   opt = (Option *)malloc(sizeof(Option));
   opt->type = OPTION;
   opt->name = "clean after install/update";
   opt->arg = config.make_option_arg[MK_OPTION_CLEAN];
   opt->state = config.make_option[MK_OPTION_CLEAN];
   add_list_item(l, opt);

   opt = (Option *)malloc(sizeof(Option));
   opt->type = OPTION;
   opt->name = "NO_CHECKSUM (don't verify checksum of source)";
   opt->arg = config.make_option_arg[MK_OPTION_NOCHKSUM];
   opt->state = config.make_option[MK_OPTION_NOCHKSUM];
   add_list_item(l, opt);

   opt = (Option *)malloc(sizeof(Option));
   opt->type = OPTION;
   opt->name = "NO_DEPENDS (don't verify build dependencies)";
   opt->arg = config.make_option_arg[MK_OPTION_NODEPS];
   opt->state = config.make_option[MK_OPTION_NODEPS];
   add_list_item(l, opt);

   opt = (Option *)malloc(sizeof(Option));
   opt->type = OPTION;
   opt->name = "FORCE_PKG_REGISTER (register port, also if it still exists)";
   opt->arg = config.make_option_arg[MK_OPTION_FORCEPKGREG];
   opt->state = config.make_option[MK_OPTION_FORCEPKGREG];
   add_list_item(l, opt);

   opt = (Option *)malloc(sizeof(Option));
   opt->type = OPTION;
   opt->name = "NO_PKG_REGISTER (don't register port, after compilation)";
   opt->arg = config.make_option_arg[MK_OPTION_NOPKGREG];
   opt->state = config.make_option[MK_OPTION_NOPKGREG];
   add_list_item(l, opt);

   /* additional port specific options */
   if (lopts != NULL) {
      itr = lopts->head;
      while (itr != NULL) {
         add_list_item(l, (Option *)itr->item);
         itr = itr->next;
      }
      free_list(lopts);
   }

   p->lopts = l;
}

/* searches in an array for next item with strcmp method,
   returns index (y) of next item, if item could not be found returns -1
   and if it's a Line also (x) the col index of s[0] */
Point
search(void *items[], int num_of_items, char *s, int start, int direction) {
   Point pt;
   char *result;

   pt.y = start;

   if (num_of_items > 0) {
      /* search from start index to end */
      for (; (direction == -1) ? pt.y >= 0 : pt.y < num_of_items; pt.y += direction) {
         if (((Port *)items[pt.y])->type == PORT) {
            /* search through port name or descr */
            if ((str_str(((Port *)items[pt.y])->name, s) != NULL) ||
                  (str_str(((Port *)items[pt.y])->descr, s) != NULL))
               return pt; /* found */
         } else if (((Category *)items[pt.y])->type == CATEGORY) {
            /* search through cat name */
            if (str_str(((Category *)items[pt.y])->name, s) != NULL) 
               return pt;
         } else if (((Option *)items[pt.y])->type == OPTION) {
            /* search through opt name */
            if (str_str(((Option *)items[pt.y])->name, s) != NULL) 
               return pt;
         } else if (((Line *)items[pt.y])->type == LINE) {
            /* search through line name */
            if ((result = str_str(((Line *)items[pt.y])->name, s)) != NULL) {
               char *ch = ((Line *)items[pt.y])->name;
               pt.x = result - ch;
               return pt;
            }
         }
      }

      /* wrapped search */
      for (pt.y = (direction == -1) ? (num_of_items - 1) : 0;
            (direction == -1) ? pt.y > start : pt.y < start; pt.y += direction) {
         if (((Port *)items[pt.y])->type == PORT) {
            /* search through port name or descr */
            if ((str_str(((Port *)items[pt.y])->name, s) != NULL) ||
                  (str_str(((Port *)items[pt.y])->descr, s) != NULL))
               return pt;
         } else if (((Category *)items[pt.y])->type == CATEGORY) {
            /* search through cat name */
            if (str_str(((Category *)items[pt.y])->name, s) != NULL) 
               return pt;
         } else if (((Option *)items[pt.y])->type == OPTION) {
            /* search through opt name */
            if (str_str(((Option *)items[pt.y])->name, s) != NULL) 
               return pt;
         } else if (((Line *)items[pt.y])->type == LINE) {
            /* search through line name */
            if ((result = str_str(((Line *)items[pt.y])->name, s)) != NULL) {
               char *ch = ((Line *)items[pt.y])->name;
               pt.x = result - ch;
               return pt;
            }
         }
      }
   }

   /* not found */
   pt.y = -1;
   return pt;
}


/* unmarks all marked dependencies of this entry, if there
   aren't any conflicts with other installed or selected
   ports */
void
unmark_all_dependencies() {
   extern List *lprts;
   Iter itr = lprts->head;

   /* this algorithm uses a fast trick: unmark all dependency ports,
      then mark dependencies of all selected ports O(2n)*/
   while (itr != NULL) {
      if ((((Port *)itr->item)->state == STATE_RDEP) ||
            (((Port *)itr->item)->state == STATE_BDEP))
         mark_port((Port *)itr->item, STATE_NOT_SELECTED, -1);
      itr = itr->next;
   }
   /* now mark all selected ports again, selected ports are ones
      with state install or update */
   itr = lprts->head;
   while (itr != NULL) {
      if ((((Port *)itr->item)->state == STATE_INSTALL) ||
            (((Port *)itr->item)->state == STATE_UPDATE))
         mark_dependencies((Port *)itr->item);
      itr = itr->next;
   }
}

/* refreshes state of a specified category */
void
refresh_cat_state(Category *cat) {
   Iter pitr;
   int num_of_deinst_ports;
   int num_of_inst_ports;
   int num_of_marked_ports;
   Port *p;

   pitr = cat->lprts->head;
   num_of_deinst_ports = 0;
   num_of_inst_ports = 0;
   num_of_marked_ports = 0;
   while (pitr != NULL) {
      if (((Port *)pitr->item)->type == PORT) {
         p = (Port *)pitr->item;
         switch (p->state) {
            case STATE_INSTALLED:
            case STATE_INSTALLED_OLDER:
            case STATE_INSTALLED_NEWER:
               num_of_inst_ports++;
               break;
            case STATE_INSTALL:
            case STATE_UPDATE:
            case STATE_BDEP:
            case STATE_RDEP:
               num_of_marked_ports++;
               break;
            case STATE_DEINSTALL:
               num_of_deinst_ports++;
               break; 
         }
      }
      pitr = pitr->next;
   }
   cat->num_of_deinst_ports = num_of_deinst_ports;
   cat->num_of_inst_ports = num_of_inst_ports;
   cat->num_of_marked_ports = num_of_marked_ports;

}

/* refreshes state of all categories (numbers of marked/inst/deinst ports */
void
refresh_cat_states() {
   extern List *lcats;
   Iter citr = lcats->head;

   while (citr != NULL) {
      refresh_cat_state((Category *)citr->item);
      citr = citr->next;
   }
}

/* marks port with state and increments all num_of_marked_ports
   items of its dedicated categories */
void
mark_port(Port *p, int st, int incrementor) {
   extern List *lcats;
   Category *metacatall = (Category *)lcats->head->item;
   Category *metacatinst = (Category *)lcats->head->next->item;
   Iter itr = p->lcats->head;

   /* lcats is modified here, because the ports doesn't "know",
      that they're also dedicated to meta category "All" */
   if (st == STATE_DEINSTALL) {
      metacatall->num_of_deinst_ports += incrementor;
      metacatinst->num_of_deinst_ports += incrementor;
      p->state = st;
      while (itr != NULL) {
         ((Category *)itr->item)->num_of_deinst_ports += incrementor;
         itr = itr->next;
      }
   } else {
      metacatall->num_of_marked_ports += incrementor;
      metacatinst->num_of_marked_ports += incrementor;
      p->state = st;
      while (itr != NULL) {
         ((Category *)itr->item)->num_of_marked_ports += incrementor;
         itr = itr->next;
      }
   }
}

/* sets the state of all dependencies of entry to 4 */
/* this function should be invoked, if the user chooses
   (i)nstall or (u)pdate or (p)ower delete */
void
mark_dependencies(Port *p) {
   Iter bitr = p->lbdep->head;
   Iter ritr = p->lrdep->head;
   Port *prt;

   /* mark run dependencies */
   while (ritr != NULL) {
      prt = (Port *)ritr->item;
      if ((prt->state == STATE_NOT_SELECTED) ||
            (prt->state == STATE_DEINSTALL)) {
         mark_port(prt, STATE_RDEP, 1);
      }
      ritr = ritr->next;
   }

   /* mark build dependencies */
   while (bitr != NULL) {
      prt = (Port *)bitr->item;
      if ((prt->state == STATE_NOT_SELECTED) ||
            (prt->state == STATE_DEINSTALL)) { 
         mark_port(prt, STATE_BDEP, 1);
      }
      bitr = bitr->next;
   }
}

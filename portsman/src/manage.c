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

/* marks all ports of list of lh with specific state, if possible */
void
mark_ports(Lhd *lh, int state) {
   Iter itr = lh->head;
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
create_filter_category(Lhd *lhfilter, char *name,
      int type, void *item) {
   Lhd *lh = (Lhd *)malloc(sizeof(Lhd));
   Category *cat = (Category *)malloc(sizeof(Category));
   Iter itr = lhfilter->head;
   Node *n = NULL;
   Port *p;

   /* init */
   lh->head = NULL;
   lh->num_of_items = 0;
   cat->num_of_ports = 0;
   cat->num_of_marked_ports = 0;
   cat->num_of_inst_ports = 0;
   cat->num_of_deinst_ports = 0;
 
   if (type == STATE) { /* filter ports through state */
      int state = *(int *)item;
      while (itr != NULL) {
         p = (Port *)itr->item;
         if (p->state == state) {
           n = add_list_item_after(lh, n, p);

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
   } else if (type == STRING) { /* filter ports through search string */
      char *s = (char *)item;
      while (itr != NULL) {
         p = (Port *)itr->item;
         if ((str_str(p->name, s) != NULL) ||
               (str_str(p->descr, s) != NULL)) {
            n = add_list_item_after(lh, n, p);

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
   }

   /* creates a fake category to rape browse_list for filter browser
      features */
   cat->type = CATEGORY;
   cat->name = strdup(name); 
   cat->num_of_ports = lh->num_of_items;
   cat->lhports = lh;
      
   return cat;
}

/* creates fake category for (de)install/update proceed browsing */
Category *
create_proceed_category() {

   extern Lhd *lhports;
   Lhd *lh = (Lhd *)malloc(sizeof(Lhd));
   Category *cat = (Category *)malloc(sizeof(Category));
   Iter pitr = lhports->head;
   Iter oitr = NULL;
   Node *n = NULL;
   Port *p;
   int num_of_deinst_ports = 0;
   
   lh->head = NULL;
   lh->num_of_items = 0;
   while (pitr != NULL) {
      p = (Port *)pitr->item;
      if ((p->state == STATE_INSTALL) ||
            (p->state == STATE_UPDATE) || 
            (p->state == STATE_DEINSTALL))  {
         n = add_list_item_after(lh, n, p);
         if (p->state != STATE_DEINSTALL) {
            if (p->lhopts == NULL) create_options(p);
            oitr = p->lhopts->head;
            /* add also all options to the list */
            while (oitr != NULL) {
               n = add_list_item_after(lh, n, (Option *)oitr->item);
               oitr = oitr->next;
            }
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
   cat->num_of_ports = lh->num_of_items;
   cat->num_of_inst_ports = 0;
   cat->num_of_marked_ports = lh->num_of_items;
   cat->num_of_deinst_ports = num_of_deinst_ports;
   cat->lhports = lh;
      
   return cat;
}

/* creates all possible options to this port */
void
create_options(Port *p) {
   Lhd *lh = (Lhd *)malloc(sizeof(Lhd));
   Lhd *lhopts = NULL;
   Option *opt;
   Iter itr;
   Node *n = NULL;
   char mkfile[MAX_PATH];

   /* init */
   lh->head = NULL;
   lh->num_of_items = 0;
   sprintf(mkfile, "%s/Makefile", p->path);
   lhopts = parse_options(mkfile);
   
   /* first of all, add standard option and make targets */
   opt = (Option *)malloc(sizeof(Option));
   opt->type = OPTION;
   opt->name = "ignore all errors while compilation of port (make -k)";
   opt->cmd = "-k";
   opt->state = STATE_NOT_SELECTED;
   n = add_list_item_after(lh, n, opt);

   opt = (Option *)malloc(sizeof(Option));
   opt->type = OPTION;
   opt->name = "generate a package of the port after install/update";
   opt->cmd = "package";
   opt->state = STATE_NOT_SELECTED;
   n = add_list_item_after(lh, n, opt);

   opt = (Option *)malloc(sizeof(Option));
   opt->type = OPTION;
   opt->name = "clean after install/update";
   opt->cmd = "clean";
   opt->state = STATE_SELECTED;
   n = add_list_item_after(lh, n, opt);

   opt = (Option *)malloc(sizeof(Option));
   opt->type = OPTION;
   opt->name = "NO_CHECKSUM (don't verify checksum of source)";
   opt->cmd = "NO_CHECKSUM=yes";
   opt->state = STATE_NOT_SELECTED;
   n = add_list_item_after(lh, n, opt);

   opt = (Option *)malloc(sizeof(Option));
   opt->type = OPTION;
   opt->name = "NO_DEPENDS (don't verify build dependencies)";
   opt->cmd = "NO_DEPENDS=yes";
   opt->state = STATE_NOT_SELECTED;
   n = add_list_item_after(lh, n, opt);

   opt = (Option *)malloc(sizeof(Option));
   opt->type = OPTION;
   opt->name = "FORCE_PKG_REGISTER (register port, also if it still exists)";
   opt->cmd = "FORCE_PKG_REGISTER=yes";
   opt->state = STATE_NOT_SELECTED;
   n = add_list_item_after(lh, n, opt);

   opt = (Option *)malloc(sizeof(Option));
   opt->type = OPTION;
   opt->name = "NO_PKG_REGISTER (don't register port, after compilation)";
   opt->cmd = "NO_PKG_REGISTER=yes";
   opt->state = STATE_NOT_SELECTED;
   n = add_list_item_after(lh, n, opt);

   /* additional port specific options */
   if (lhopts != NULL) {
      itr = lhopts->head;
      while (itr != NULL) {
         n = add_list_item_after(lh, n, (Option *)itr->item);
         itr = itr->next;
      }
      free_list(lhopts);
   }

   p->lhopts = lh;
}

/* searches in an array for next item with strcmp method,
   returns index of next item, if item could not be found returns -1 */
int
search(void *items[], int num_of_items, char *s, int start, int direction) {
   int i = start;

   if (num_of_items > 0) {
      /* search from start index to end */
      for (; (direction == -1) ? i >= 0 : i < num_of_items; i += direction) {
         if (((Port *)items[i])->type == PORT) {
            /* search through port name or descr */
            if ((str_str(((Port *)items[i])->name, s) != NULL) ||
                  (str_str(((Port *)items[i])->descr, s) != NULL))
               return i; /* found */
         } else if (((Category *)items[i])->type == CATEGORY) {
            /* search through cat name */
            if (str_str(((Category *)items[i])->name, s) != NULL) 
               return i;
         } else if (((Option *)items[i])->type == OPTION) {
            /* search through opt name */
            if (str_str(((Option *)items[i])->name, s) != NULL) 
               return i;
         }
      }

      /* wrapped search */
      for (i = (direction == -1) ? (num_of_items - 1) : 0;
            (direction == -1) ? i > start : i < start; i += direction) {
         if (((Port *)items[i])->type == PORT) {
            /* search through port name or descr */
            if ((str_str(((Port *)items[i])->name, s) != NULL) ||
                  (str_str(((Port *)items[i])->descr, s) != NULL))
               return i;
         } else if (((Category *)items[i])->type == CATEGORY) {
            /* search through cat name */
            if (str_str(((Category *)items[i])->name, s) != NULL) 
               return i;
         } else if (((Option *)items[i])->type == OPTION) {
            /* search through opt name */
            if (str_str(((Option *)items[i])->name, s) != NULL) 
               return i;
         }
      }
   }

   /* not found */
   return -1;
}


/* unmarks all marked dependencies of this entry, if there
   aren't any conflicts with other installed or selected
   ports */
void
unmark_all_dependencies() {
   extern Lhd *lhports;
   Iter itr = lhports->head;

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
   itr = lhports->head;
   while (itr != NULL) {
      if ((((Port *)itr->item)->state == STATE_INSTALL) ||
            (((Port *)itr->item)->state == STATE_UPDATE))
         mark_dependencies((Port *)itr->item);
      itr = itr->next;
   }
}

/* marks port with state and increments all num_of_marked_ports
   items of its dedicated categories */
void
mark_port(Port *p, int st, int incrementor) {
   extern State state;
   extern Lhd *lhcats;
   Category *metacat = (Category *)lhcats->head->item;
   Iter itr = p->lhcats->head;

   /* lhcats is modified here, because the ports doesn't "know",
      that they're also dedicated to meta category "All" */
   if (st == STATE_DEINSTALL) {
      state.num_of_deinst_ports += incrementor;
      metacat->num_of_deinst_ports += incrementor;
      p->state = st;
      while (itr != NULL) {
         ((Category *)itr->item)->num_of_deinst_ports += incrementor;
         itr = itr->next;
      }
   } else {
      state.num_of_marked_ports += incrementor;
      metacat->num_of_marked_ports += incrementor;
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
   Iter bitr = p->lhbdep->head;
   Iter ritr = p->lhrdep->head;
	Port *prt;
   
   /* mark build dependencies */
   while (bitr != NULL) {
		prt = (Port *)bitr->item;
		if ((prt->state == STATE_NOT_SELECTED) ||
				(prt->state == STATE_DEINSTALL)) {
			mark_port(prt, STATE_BDEP, 1);
		}
		bitr = bitr->next;
	}

   /* mark run dependencies */
   while (ritr != NULL) {
		prt = (Port *)ritr->item;
		if ((prt->state == STATE_NOT_SELECTED) ||
				(prt->state == STATE_DEINSTALL)) {
			mark_port(prt, STATE_RDEP, 1);
		}
		ritr = ritr->next;
	}
}

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

/* marks all ports of list l with specific state, if possible */
void
mark_ports(List *l, int state) {
   Node *n = l->head;
   Port *port;

   while (n != NULL) {
      port = (Port *)n->item;
      switch (state) {
         case STATE_INSTALL:
            if (port->state == STATE_NOT_SELECTED) {
               mark_port(port, STATE_INSTALL, 1);
               mark_dependencies(port);
            }
         break;
         case STATE_UPDATE:
            if ((port->state == STATE_INSTALLED) ||
                  (port->state == STATE_INSTALLED_OLDER)) {
               mark_port(port, STATE_UPDATE, 1);
               mark_dependencies(port);
            }
         break;
         case STATE_DEINSTALL:
            if (port->state >= STATE_INSTALLED) {
               mark_port(port, STATE_DEINSTALL, 1);
            }
			case STATE_NOT_SELECTED:
				if ((port->state == STATE_INSTALL) ||
						(port->state == STATE_UPDATE)) {
					mark_port(port, STATE_NOT_SELECTED, -1);
				}
         break;   
      }
      n = n->next;
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
   Node *n = lfilter->head;
   Node *p = NULL;
   Port *port;

   /* init */
   l->head = NULL;
   l->num_of_items = 0;
   cat->num_of_ports = 0;
   cat->num_of_marked_ports = 0;
   cat->num_of_inst_ports = 0;
   cat->num_of_deinst_ports = 0;
 
   if (type == STATE) { /* filter ports through state */
      int state = *(int *)item;
      while (n != NULL) {
         port = (Port *)n->item;
         if (port->state == state) {
           p = add_list_item_after(l, p, port);

           switch (port->state) {
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
         n = n->next;
      } 
   } else if (type == STRING) { /* filter ports through search string */
      char *s = (char *)item;
      while (n != NULL) {
         port = (Port *)n->item;
         if ((str_str(port->name, s) != NULL) ||
               (str_str(port->descr, s) != NULL)) {
            p = add_list_item_after(l, p, port);

            switch (port->state) {
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
         n = n->next;
      }
   }

   /* creates a fake category to rape browse_list for filter browser
      features */
   cat->type = CATEGORY;
   cat->name = strdup(name); 
   cat->num_of_ports = l->num_of_items;
   cat->lports = l;
      
   return cat;
}

/* creates fake category for (de)install/update proceed browsing */
Category *
create_proceed_category() {

   extern List *lports;
   List *l = (List *)malloc(sizeof(List));
   Category *cat = (Category *)malloc(sizeof(Category));
   Node *n = lports->head;
   Node *o = NULL;
   Node *p = NULL;
   Port *port;
   int num_of_deinst_ports = 0;
   
   l->head = NULL;
   l->num_of_items = 0;
   while (n != NULL) {
      port = (Port *)n->item;
      if ((port->state == STATE_INSTALL) ||
            (port->state == STATE_UPDATE) || 
            (port->state == STATE_DEINSTALL))  {
         p = add_list_item_after(l, p, port);
         if (port->state != STATE_DEINSTALL) {
            if (port->lopts == NULL) create_options(port);
            o = port->lopts->head;
            /* add also all options to the list */
            while (o != NULL) {
               p = add_list_item_after(l, p, (Option *)o->item);
               o = o->next;
            }
         } else {
            num_of_deinst_ports++;
         }
      }   
      n = n->next;
   }

   /* creates a fake category to rape browse_list for proceed browser
      features */
   cat->type = CATEGORY;
   cat->name = "(de)installation/upgrade"; 
   cat->num_of_ports = l->num_of_items;
   cat->num_of_inst_ports = 0;
   cat->num_of_marked_ports = l->num_of_items;
   cat->num_of_deinst_ports = num_of_deinst_ports;
   cat->lports = l;
      
   return cat;
}

/* creates all possible options to this port */
void
create_options(Port *port) {
   List *l = (List *)malloc(sizeof(List));
   List *lopts = NULL;
   Option *opt;
   Node *prev = NULL;
   Node *n;
   char mkfile[MAX_PATH];

   /* init */
   l->head = NULL;
   l->num_of_items = 0;
   sprintf(mkfile, "%s/Makefile", port->path);
   lopts = parse_options(mkfile);
   
   /* first of all, add standard option and make targets */
   opt = (Option *)malloc(sizeof(Option));
   opt->type = OPTION;
   opt->name = "ignore all errors while compilation of port (make -k)";
   opt->cmd = "-k";
   opt->state = STATE_NOT_SELECTED;
   prev = add_list_item_after(l, prev, opt);

   opt = (Option *)malloc(sizeof(Option));
   opt->type = OPTION;
   opt->name = "generate a package of the port after install/update";
   opt->cmd = "package";
   opt->state = STATE_NOT_SELECTED;
   prev = add_list_item_after(l, prev, opt);

   opt = (Option *)malloc(sizeof(Option));
   opt->type = OPTION;
   opt->name = "clean after install/update";
   opt->cmd = "clean";
   opt->state = STATE_SELECTED;
   prev = add_list_item_after(l, prev, opt);

   opt = (Option *)malloc(sizeof(Option));
   opt->type = OPTION;
   opt->name = "NO_CHECKSUM (don't verify checksum of source)";
   opt->cmd = "NO_CHECKSUM=yes";
   opt->state = STATE_NOT_SELECTED;
   prev = add_list_item_after(l, prev, opt);

   opt = (Option *)malloc(sizeof(Option));
   opt->type = OPTION;
   opt->name = "NO_DEPENDS (don't verify build dependencies)";
   opt->cmd = "NO_DEPENDS=yes";
   opt->state = STATE_NOT_SELECTED;
   prev = add_list_item_after(l, prev, opt);

   opt = (Option *)malloc(sizeof(Option));
   opt->type = OPTION;
   opt->name = "FORCE_PKG_REGISTER (register port, also if it still exists)";
   opt->cmd = "FORCE_PKG_REGISTER=yes";
   opt->state = STATE_NOT_SELECTED;
   prev = add_list_item_after(l, prev, opt);

   opt = (Option *)malloc(sizeof(Option));
   opt->type = OPTION;
   opt->name = "NO_PKG_REGISTER (don't register port, after compilation)";
   opt->cmd = "NO_PKG_REGISTER=yes";
   opt->state = STATE_NOT_SELECTED;
   prev = add_list_item_after(l, prev, opt);

   /* additional port specific options */
   if (lopts != NULL) {
      n = lopts->head;
      while (n != NULL) {
         prev = add_list_item_after(l, prev, (Option *)n->item);
         n = n->next;
      }
      free_list(lopts);
   }

   port->lopts = l;
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
   extern List *lports;
   Node *n = lports->head;

   /* this algorithm uses a fast trick: unmark all dependency ports,
      then mark dependencies of all selected ports O(2n)*/
   while (n != NULL) {
      if ((((Port *)n->item)->state == STATE_RDEP) ||
            (((Port *)n->item)->state == STATE_BDEP))
         mark_port((Port *)n->item, STATE_NOT_SELECTED, -1);
      n = n->next;
   }
   /* now mark all selected ports again, selected ports are ones
      with state install or update */
   n = lports->head;
   while (n != NULL) {
      if ((((Port *)n->item)->state == STATE_INSTALL) ||
            (((Port *)n->item)->state == STATE_UPDATE))
         mark_dependencies((Port *)n->item);
      n = n->next;
   }
}

/* marks port with state and increments all num_of_marked_ports
   items of its dedicated categories */
void
mark_port(Port *port, int st, int incrementor) {
   extern State state;
   extern List *lcats;
   Category *metacat = (Category *)lcats->head->item;
   Node *n = port->lcats->head;

   /* lcats is modified here, because the ports doesn't "know",
      that they're also dedicated to meta category "All" */
   if (st == STATE_DEINSTALL) {
      state.num_of_deinst_ports += incrementor;
      metacat->num_of_deinst_ports += incrementor;
      port->state = st;
      while (n != NULL) {
         ((Category *)n->item)->num_of_deinst_ports += incrementor;
         n = n->next;
      }
   } else {
      state.num_of_marked_ports += incrementor;
      metacat->num_of_marked_ports += incrementor;
      port->state = st;
      while (n != NULL) {
         ((Category *)n->item)->num_of_marked_ports += incrementor;
         n = n->next;
      }
   }
}

/* sets the state of all dependencies of entry to 4 */
/* this function should be invoked, if the user chooses
   (i)nstall or (u)pdate or (p)ower delete */
void
mark_dependencies(Port *port) {
   Node *bnode = port->lbdep->head;
   Node *rnode = port->lrdep->head;
	Port *p;
   
   /* mark build dependencies */
   while (bnode != NULL) {
		p = (Port *)bnode->item;
		if ((p->state == STATE_NOT_SELECTED) ||
				(p->state == STATE_DEINSTALL)) {
			mark_port(p, STATE_BDEP, 1);
		}
		bnode = bnode->next;
	}

   /* mark run dependencies */
   while (rnode != NULL) {
		p = (Port *)rnode->item;
		if ((p->state == STATE_NOT_SELECTED) ||
				(p->state == STATE_DEINSTALL)) {
			mark_port(p, STATE_RDEP, 1);
		}
		rnode = rnode->next;
	}
}

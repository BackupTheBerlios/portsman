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

void
set_option_titlestatus(Port *port, int top, int bottom) {
   char buf[80];
   sprintf(buf, " [compile options of %-.40s]", port->name);
   wprint_titlebar(buf);
   sprintf(buf, " (%3d-%3d /%3d) options", top, bottom,
         port->lopts->num_of_items);
   wprint_statusbar(buf);
   doupdate();
}

void
set_ports_titlestatus(Category *cat, int top, int bottom, bool proceed) {
   char buf[80];
   sprintf(buf, " [%s]", cat->name);
   wprint_titlebar(buf);
	if (!proceed)
		sprintf(buf, " (%5d-%5d) port(s)          -%3d/ +%3d/%5d/%5d port(s)",
				top, bottom,
            cat->num_of_deinst_ports, cat->num_of_marked_ports,
            cat->num_of_inst_ports, cat->num_of_ports);
	else
		sprintf(buf, " (%5d-%5d) item(s)          -%3d/ +%3d/%5d/%5d item(s)",
				top, bottom,
            cat->num_of_deinst_ports, cat->num_of_marked_ports,
            cat->num_of_inst_ports, cat->num_of_ports);
   wprint_statusbar(buf);
   doupdate();
}

void
set_cat_titlestatus(int top, int bottom) {
   extern State state;
   char buf[80];
   wprint_titlebar(" [categories]");
   sprintf(buf, " (%3d-%3d/%3d) categories       -%3d/ +%3d/%5d/%5d ports",
         top, bottom, state.num_of_cats, state.num_of_deinst_ports,
         state.num_of_marked_ports, state.num_of_inst_ports,
         state.num_of_ports);
   wprint_statusbar(buf);
   doupdate();
}

/* manipulates indexes for forward search */
void
fw_search(int num_of_items, int nextidx, int maxy,
      int *topidx, int *curridx) {
   if (nextidx < *topidx + *curridx) { /* search wrapped at bottom */
      wprint_cmdinfo(" search wrapped at bottom");
      doupdate();

      if (nextidx - *curridx < 0) {
         *topidx = 0;
         *curridx = nextidx;
      } else {
         *topidx = nextidx - *curridx;
      }

   } else {
      if ((nextidx - *curridx + maxy) < num_of_items) {
         *topidx = nextidx - *curridx;
      } else {
         *topidx = num_of_items - maxy; 
         *curridx = nextidx - *topidx;
      }
   }
}

/* manipulates indexes for backward search */
void
bw_search(int num_of_items, int nextidx, int maxy,
      int *topidx, int *curridx) {
   if (nextidx < *topidx + *curridx) { /* search wrapped at bottom */
      if (nextidx - *curridx < 0) {
         *topidx = 0;
         *curridx = nextidx;
      } else {
         *topidx = nextidx - *curridx;
      }

   } else {
     wprint_cmdinfo(" search wrapped at top ");
     doupdate();

     if ((nextidx - *curridx + maxy) < num_of_items) {
         *topidx = nextidx - *curridx;
      } else {
         *topidx = num_of_items - maxy; 
         *curridx = nextidx - *topidx;
      }
   }
}

/* browses through proceed actions */
void
browse_proceed() {

   extern bool redraw_dimensions;
   Category *cat;
   int input;

	wprint_cmdinfo(" Please wait while scanning all compile options...");
	doupdate();
   
   cat = create_proceed_category();
      
   if (browse_list(cat->lports, cat, TRUE) > 0)
      redraw_dimensions = TRUE;
   
   input = wprint_inputoutput_ch("Proceed with (de)installation/upgrade of above ports? [y/n] ");
   if (input == 'y') {
      proceed_action(cat->lports);
   }
   wprint_cmdinfo("");
   doupdate();

   free_list(cat->lports);
   free(cat);
}

/* browses (through) file (path) content */
/* Note: MAXX argument is used to determine the buffersize and
   horizontal displaysize of the file content. */
int
browse_file(char *path) {
   extern WINDOW *wbrowse;
   extern bool redraw_dimensions;
   FILE *fd;
   char buf[wbrowse->_maxx];
   register int i = 0;
   int maxy, maxx;
   int press;

   wprint_cmdinfo("");
   doupdate();
   wclear(wbrowse);
   getmaxyx(wbrowse, maxy, maxx);

   /* return if file couldn't be opened */
   if ((fd = fopen(path, "r")) == NULL) return (-1);
   sprintf(buf, " [%-.60s]", path);
   wprint_titlebar(buf);
         
   while (feof(fd) == 0) {
      if (fgets(buf, maxx, fd) != NULL)
         mvwprintw(wbrowse, i++, 0, buf);
      else
         break;
      if (i == maxy) {
         wprint_statusbar(" press any key for next page or (q)uit to exit");
         doupdate();
         press = wgetch(wbrowse);
         if (redraw_dimensions) return (-1);
         switch (press) {
            case 'q':
               return (-1);
               break;
            default:
               i = 0;
               wclear(wbrowse);
               break;
         } 
      }
   }
   fclose(fd);

   /* finally display everything */
   wprint_statusbar(" press any key to proceed");
   doupdate();
   press = wgetch(wbrowse);
   wclear(wbrowse);

   return (0);
}

/* browses a port summary */
void
browse_port_summary(Port *port) {
   extern WINDOW *wbrowse;
   int y = 0;
   Node *cnode = port->lcats->head;
	Node *bnode = port->lbdep->head;
	Node *rnode = port->lrdep->head;
   Node *fnode;
   List *fl;
   char msg[80];
   char plistfile[MAX_PATH];

   /* init */
   wprint_cmdinfo("");
   doupdate();

   if (port != NULL) {
      sprintf(msg, " [%-.60s]", port->name);
      wprint_titlebar(msg);
      wprint_statusbar(" press any key to proceed");
      doupdate();
      wclear(wbrowse);

      sprintf(msg, "Path to port       : %-.50s", port->path);
      if (wprint_line(wbrowse, &y, 0, msg, TRUE) < 0) return;
      sprintf(msg, "Installation prefix: %-.50s", port->instpfx);
      if (wprint_line(wbrowse, &y, 0, msg, TRUE) < 0) return; 
      sprintf(msg, "Description        : %-.50s", port->descr);
      if (wprint_line(wbrowse, &y, 0, msg, TRUE) < 0) return;
      sprintf(msg, "Maintainer         : %-.50s", port->maintainer);
      if (wprint_line(wbrowse, &y, 0, msg, TRUE) < 0) return;
      while (cnode != NULL) {
         sprintf(msg, "Category           : %-.50s",
               ((Category *)cnode->item)->name);
         if (wprint_line(wbrowse, &y, 0, msg, TRUE) < 0) return;
         cnode = cnode->next;
      }
      while (bnode != NULL) {
			Port *p = (Port *)bnode->item;
         sprintf(msg, "Build dependency   : %-30.30s (%-.20s)",
					p->name, (p->state >= STATE_INSTALLED) ? "installed":
					"not installed");
         if (wprint_line(wbrowse, &y, 0, msg, TRUE) < 0) return;
         bnode = bnode->next;
      }
      while (rnode != NULL) {
			Port *p = (Port *)rnode->item;
         sprintf(msg, "Run dependency     : %-30.30s (%-.20s)",
					p->name, (p->state >= STATE_INSTALLED) ? "installed":
					"not installed");
         if (wprint_line(wbrowse, &y, 0, msg, TRUE) < 0) return;
         rnode = rnode->next;
      }
      sprintf(msg, "Homepage           : %-.50s", port->url);
      if (wprint_line(wbrowse, &y, 0, msg, TRUE) < 0) return;

      /* init plistfile */
      sprintf(plistfile, "%s/pkg-plist", port->path);
      fl = parse_plist(port, plistfile);
      fnode = fl->head;
      while (fnode != NULL) {
         Plist *p = (Plist *)fnode->item;
         sprintf(msg, "file of port       : %-30.30s (%-.20s)",
               p->name, (p->exist == TRUE) ? "installed" : "not installed");
         if (wprint_line(wbrowse, &y, 0, msg, TRUE) < 0) return;
         fnode = fnode->next;
      }
      free_list(fl);
      wgetch(wbrowse);
      wclear(wbrowse);
   }
}

/* generic list browser, prev means parent item, if exists */
int
browse_list(List *l, void *parent, bool proceed) {
   extern WINDOW *wbrowse;
   extern State state;
   extern List *lports;
   extern bool redraw_dimensions;
   void *items[l->num_of_items];
   int press;
   int redraw;
   int maxy, maxx;
   int topidx  = 0; /* index of visible top item */
   int curridx = 0; /* index of currently selected item */
   int previdx = 0; /* index of previously selected item */
   int i, nextidx = 0, lastidx = 0;
   int expch = 0;
   int result = 0;
   char *expstr = NULL;
   char msg[80];
   List *lfilter;
   Category *cat = NULL;

   if (l->num_of_items < 1) /* return immediately, if there aren't
                                  any items */
      return (-1); 

   /* initialization */
   create_array_from_list(l, items); 
   wprint_cmdinfo("");
   doupdate();
   wclear(wbrowse);
   getmaxyx(wbrowse, maxy, maxx);
   /* trick for categories with fewer than maxy items */
   if (l->num_of_items < maxy) maxy = l->num_of_items;
   noecho();
   curs_set(0); /* hide cursor */
   keypad(wbrowse, TRUE);
   redraw = REFRESH_WINDOW;

   do {
      /* at start up of browse_list and when SIGWINCH was
         received */
      if (redraw_dimensions) {
         wprint_cmdinfo("");
         doupdate();
         wclear(wbrowse);
         getmaxyx(wbrowse, maxy, maxx);
         /* trick for categories with fewer than maxy items */
         if (l->num_of_items < maxy) maxy = l->num_of_items;
         noecho();
         curs_set(0); /* hide cursor */
         keypad(wbrowse, TRUE);

         /* redraw dimensions until first browse_list instance was reached,
            this is done through a positive return value */
         result = 1; 
         redraw_dimensions = FALSE;
         redraw = REFRESH_WINDOW;
      }
 
      /* set title status */
      if (((Category *)items[0])->type == CATEGORY) {
         set_cat_titlestatus(topidx, topidx + maxy);
      } else if (((Port *)items[0])->type == PORT) {
         /* here parent is needed */
         set_ports_titlestatus((Category *)parent, topidx,
					topidx + maxy, proceed);
      } else if (((Option *)items[0])->type == OPTION) {
         set_option_titlestatus((Port *)parent, topidx, topidx + maxy);
      }

      /* begin of redraw part */
      switch (redraw) {
        case REFRESH_WINDOW:
            for (i = 0; i < maxy; i++) {
               wprint_item(wbrowse, i, 0, items[topidx + i]);
               if (curridx == i)
                  mvwchgat(wbrowse, curridx, 0, -1,
                        COLOR_PAIR(CLR_SELECTOR + 1), 0, NULL);
            }
         break;
         case REFRESH_ENTRY:
            wprint_item(wbrowse, previdx, 0, items[topidx + previdx]);
            mvwchgat(wbrowse, previdx, 0, -1,
                  COLOR_PAIR(CLR_BROWSE + 1), 0, NULL);
            wprint_item(wbrowse, curridx, 0, items[topidx + curridx]);
            mvwchgat(wbrowse, curridx, 0, -1,
                  COLOR_PAIR(CLR_SELECTOR + 1), 0, NULL);
        break;
      }
      /* end of redraw */

      /* begin of key press handling */      
      previdx = curridx;
      press = wgetch(wbrowse);
      switch (press) {
         case 'j':
         case KEY_DOWN:
               if (curridx < maxy - 1) {
                  curridx++;
                  redraw = REFRESH_ENTRY;
               } else if ((topidx + maxy) < l->num_of_items) {
                  topidx++;
                  redraw = REFRESH_WINDOW;
               }
         break;
         case 'k':
         case KEY_UP:
               if (curridx > 0) {
                  curridx--;
                  redraw = REFRESH_ENTRY;
               } else if (topidx > 0) {
                  topidx--;
                  redraw = REFRESH_WINDOW;
               }
         break;
         case 'g': /* top */
            topidx = curridx = 0;
            redraw = REFRESH_WINDOW;
         break;
         case 'G': /* bottom */
            topidx = l->num_of_items - maxy; 
            curridx = maxy - 1;   
            redraw = REFRESH_WINDOW;
         break;
         case KEY_NPAGE: /* pg_down */
            if ((topidx + (2 * maxy)) < l->num_of_items)
               topidx += maxy;  
            else {
               topidx = l->num_of_items - maxy;
               curridx = maxy - 1;
            }
            redraw = REFRESH_WINDOW;
         break;   
         case KEY_PPAGE: /* pg_up */
            if ((topidx - maxy) > 0)
               topidx -= maxy;
            else 
               topidx = curridx = 0;
            redraw = REFRESH_WINDOW;
         break;
         case 'l':
         case KEY_RIGHT:
         case '\n': /* ENTER */
            if (((Category *)items[topidx + curridx])->type == CATEGORY) {
               if (browse_list(((Category *)items[topidx + curridx])->lports,
                           items[topidx + curridx], FALSE) > 0)
                  redraw_dimensions = TRUE;
               else
                  redraw = REFRESH_WINDOW;
            } else if (((Port *)items[topidx + curridx])->type == PORT) {
               browse_file(((Port *)items[topidx + curridx])->pathpkgdesc);
               redraw = REFRESH_WINDOW;
            }
         break; 
         case KEY_F(1):
         case 'h': /* help */
            browse_file(HELP_FILE);
            redraw = REFRESH_WINDOW;
         break;   
         case '/': /* fw search */
            expstr = wprint_inputoutput_str(" forward search key: ");
            lastidx = nextidx = search(items, l->num_of_items, expstr, 
                  topidx + curridx, 1);
            if (nextidx != -1) { /* found */
               fw_search(l->num_of_items, nextidx, maxy, &topidx, &curridx);
               redraw = REFRESH_WINDOW;
            } else {
               wprint_cmdinfo(" no (more) items found in bottom direction");
               doupdate();
            }
         break;
         case 'n': /* fw search next */
            if (expstr != NULL) {
               wprint_cmdinfo("");
               doupdate();
               if (topidx + curridx != nextidx)
                  nextidx = search(items, l->num_of_items, expstr, 
                        topidx + curridx, 1);
               else {
                  if (nextidx + 1 < l->num_of_items)
                     nextidx = search(items, l->num_of_items, expstr,
                           nextidx + 1, 1);
               }

               if ((nextidx != -1) && (lastidx != nextidx)) { /* found */
                  fw_search(l->num_of_items, nextidx, maxy, &topidx, &curridx);
                  lastidx = nextidx;
                  redraw = REFRESH_WINDOW;
               } else {
                  wprint_cmdinfo(" no (more) items found in bottom direction");
                  doupdate();
               }
            }
         break;
         case '?': /* bw search */
            expstr = wprint_inputoutput_str(" backward search key: ");
            lastidx = nextidx = search(items, l->num_of_items, expstr, 
                  topidx + curridx, -1);
            if (nextidx != -1) { /* found */
               bw_search(l->num_of_items, nextidx, maxy, &topidx, &curridx);
               redraw = REFRESH_WINDOW;
            } else {
               wprint_cmdinfo(" no (more) items found in top direction");
               doupdate();
            }
         break;
         case 'N': /* bw search next */
            if (expstr != NULL) {
               wprint_cmdinfo("");
               doupdate();
               if (topidx + curridx != nextidx)
                  nextidx = search(items, l->num_of_items, expstr, 
                        topidx + curridx, -1);
               else {
                  if (nextidx - 1 >= 0)
                     nextidx = search(items, l->num_of_items, expstr,
                           nextidx - 1, -1);
               }
               if ((nextidx != -1) && (lastidx != nextidx)) { /* found */
                  bw_search(l->num_of_items, nextidx, maxy, &topidx, &curridx);
                  lastidx = nextidx;
                  redraw = REFRESH_WINDOW;
               } else {
                  wprint_cmdinfo(" no (more) items found in top direction");
                  doupdate();
               }
            }
         break;
         case 'p': /* proceed action */
            if (!proceed) {
               if ((state.num_of_marked_ports > 0) ||
                     (state.num_of_deinst_ports > 0)) {
                  browse_proceed();
                  redraw = REFRESH_WINDOW;
               } else {
                  wprint_cmdinfo(" You don't have selected any ports to (de)install or upgrade");
                  doupdate();
               }
            } else
               press = 'q';
         break;
         case 'f': /* filter */
            i = 0;
            expch =
               wprint_inputoutput_ch(" Filter ports by (s)tate or (k)eyword? [s/k] ");
            if (((Port *)items[0])->type == PORT) 
               lfilter = l;
            else
               lfilter = lports;
            if (expch == 's') {
               int state;
               expch = wprint_inputoutput_ch(" Which state? [</>/=/B/R/i/u/d] ");

               switch (expch) {
                  case '<':
                     state = STATE_INSTALLED_OLDER;
                     cat = create_filter_category(lfilter,
                           "filter of older than installed port(s)", STATE, &state);
                  break;   
                  case '>':
                     state = STATE_INSTALLED_NEWER;
                     cat = create_filter_category(lfilter,
                           "filter of newer than installed port(s)", STATE, &state);
                  break;
                  case '=':
                     state = STATE_INSTALLED;   
                     cat = create_filter_category(lfilter,
                           "filter of installed port(s)", STATE, &state);
                  break;
                  case 'B':
                     state = STATE_BDEP;   
                     cat = create_filter_category(lfilter,
                           "filter of build dependencies", STATE, &state);
                  break;
                  case 'R':
                     state = STATE_RDEP;   
                     cat = create_filter_category(lfilter,
                           "filter of run dependencies", STATE, &state);
                  break;
                  case 'i':
                     state = STATE_INSTALL;   
                     cat = create_filter_category(lfilter,
                           "filter of marked ports for installation", STATE, &state);
                  break;
                  case 'u':
                     state = STATE_UPDATE;   
                     cat = create_filter_category(lfilter,
                           "filter of marked ports for update", STATE, &state);
                  break;
                  case 'd':
                     state = STATE_DEINSTALL;   
                     cat = create_filter_category(lfilter,
                           "filter of marked ports for deinstallation", STATE, &state);
                  break;
                  default: /* error, don't know */
                     i = -1;
                  break;
               }
            } else if (expch == 'k') {
               expstr = wprint_inputoutput_str(" keyword: ");
               sprintf(msg, "filter of ports with keyword '%-.20s'", expstr);
               cat = create_filter_category(lfilter, msg, STRING, expstr);
            } else
					i = -1;
            if (i != 0) {
               wprint_cmdinfo(" No valid filter");
               doupdate();
            } else { /* everything valid */
               i = browse_list(cat->lports, cat, FALSE);
               if (i < 0) 
                  wprint_cmdinfo(" There aren't any matching ports");
               else if (i > 0)
                  redraw_dimensions = TRUE;
               else
                  wprint_cmdinfo("");
               doupdate();

               free(cat->name);
               free_list(cat->lports);
               free(cat);
               redraw = REFRESH_WINDOW;
            }
         break;
     }
      /* special key press handling for ports brwosing */
      if (((Port *)items[topidx + curridx])->type == PORT) {
         Port *p = (Port *)items[topidx + curridx];
         switch (press) {
            case KEY_BACKSPACE:
            case KEY_LEFT:
               press = 'q';
            break;   
            case 's': /* show summary */
               browse_port_summary(p);
               redraw = REFRESH_WINDOW;
            break;   
            case 'i': /* install */
               if (p->state == STATE_NOT_SELECTED) {
                  mark_port(p, STATE_INSTALL, 1);
                  mark_dependencies(p);
                  redraw = REFRESH_WINDOW;
               }
            break;
            case 'u': /* update */
               if ((p->state == STATE_INSTALLED)
                     || (p->state == STATE_INSTALLED_OLDER)) {
                  mark_port(p, STATE_UPDATE, 1);
                  mark_dependencies(p);
                  redraw = REFRESH_WINDOW;
               }
            break; 
            case 'd': /* delete */
               if (p->state >= STATE_INSTALLED) {
                  mark_port(p, STATE_DEINSTALL, 1);
                  unmark_all_dependencies(); /* remarks all other
                                                dependencies */
                  redraw = REFRESH_WINDOW;
               } 
            break; 
            case ' ': /* deselect */
               if ((p->state == STATE_INSTALL) || (p->state == STATE_UPDATE)) {
                  mark_port(p, STATE_NOT_SELECTED, -1);
                  unmark_all_dependencies();
                  redraw = REFRESH_WINDOW;
               } else if (p->state == STATE_NOT_SELECTED) { /* select for
                                                               install */
                  mark_port(p, STATE_INSTALL, 1);
                  mark_dependencies(p);
                  redraw = REFRESH_WINDOW;
               }
            break;   
            case 'o': /* port compile options */
               if (!proceed) {
                  if ((p->state == STATE_INSTALL) || (p->state == STATE_UPDATE)) {
                     if (p->lopts == NULL)
                        create_options(p);
                     if (browse_list(p->lopts, p, FALSE) > 0)
                        redraw_dimensions = TRUE;
                     else
                        redraw = REFRESH_WINDOW;
                  }
               }
					break;
				case 'm': /* marker */
					i = 0;
					expch = wprint_inputoutput_ch(" Mark all ports (if possible) with state [i/u/d/SPACE]: ");
					switch (expch) {
						case 'i':
							mark_ports(l, STATE_INSTALL);
							redraw = REFRESH_WINDOW;
							break;
						case 'u':
							mark_ports(l, STATE_UPDATE);
							redraw = REFRESH_WINDOW;
							break;
						case 'd':
							mark_ports(l, STATE_DEINSTALL);
							redraw = REFRESH_WINDOW;
							break;
						case ' ':
							mark_ports(l, STATE_NOT_SELECTED);
							redraw = REFRESH_WINDOW;
							break;	
						default:
							i = -1;
							break;
					}
					if (i != 0)
						wprint_cmdinfo(" No valid mark");
					else
						wprint_cmdinfo("");
					doupdate();
				break;   
			}
      } else if (((Option *)items[topidx + curridx])->type == OPTION) {
         Option *opt = (Option *)items[topidx + curridx];
         switch (press) {
            case ' ': /* toggle */
               opt->state = (opt->state == STATE_NOT_SELECTED) ?
                  STATE_SELECTED : STATE_NOT_SELECTED;
               redraw = REFRESH_ENTRY;
            break;
            case KEY_BACKSPACE:
            case KEY_LEFT:
               press = 'q';
            break;
         }
      }
 
      /* end of key press handling */
   } while (press != 'q'); /* (q)uit browser */
   wclear(wbrowse);
   return (result);
}

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
set_line_titlestatus(char *title, int top, int bottom, int lines) {
   char buf[MAX_COLS];
   sprintf(buf, " [%-.60s]", title);
   wprint_titlebar(buf);
   sprintf(buf, " (%3d-%3d /%4d) line(s)", top, bottom, lines);
   wprint_statusbar(buf);
   doupdate();
}

void
set_option_titlestatus(Port *p, int top, int bottom) {
   char buf[MAX_COLS];
   sprintf(buf, " [compile options of %-.40s]", p->name);
   wprint_titlebar(buf);
   sprintf(buf, " (%3d-%3d /%3d) options", top, bottom,
         p->lhopts->num_of_items);
   wprint_statusbar(buf);
   doupdate();
}

void
set_ports_titlestatus(Category *cat, int top, int bottom, bool proceed) {
   char buf[MAX_COLS];
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
   char buf[MAX_COLS];
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

   if (browse_list(cat->lhprts, cat, TRUE) > 0)
      redraw_dimensions = TRUE;

   input = wprint_inputoutput_ch("Proceed with (de)installation/upgrade of above ports? [y/n] ");
   if (input == 'y') {
      proceed_action(cat->lhprts);
   }
   wprint_cmdinfo("");
   doupdate();

   free_list(cat->lhprts);
   free(cat);
}

/* browses a port summary */
void
browse_port_summary(Port *p) {
   extern bool redraw_dimensions;
   Iter itr = p->lhcats->head;
   Lhd *lhplist;
   Lhd *lhitems = (Lhd *)malloc(sizeof(Lhd));
   Port *prt;
   Node *n = NULL;
   char msg[MAX_COLS];
   char plistfile[MAX_PATH];

   /* init */
   lhitems->head = NULL;
   lhitems->num_of_items = 0;

   if (p != NULL) {
      sprintf(msg, "Path to port: %-.60s", p->path);
      n = add_list_item_after(lhitems, n, create_line(msg));
      sprintf(msg, "Inst. prefix: %-.60s", p->instpfx);
      n = add_list_item_after(lhitems, n, create_line(msg));
      sprintf(msg, "Description : %-.60s", p->descr);
      n = add_list_item_after(lhitems, n, create_line(msg));
      sprintf(msg, "Maintainer  : %-.60s", p->maintainer);
      n = add_list_item_after(lhitems, n, create_line(msg));
      while (itr != NULL) {
         sprintf(msg, "Category    : %-.50s",
               ((Category *)itr->item)->name);
         n = add_list_item_after(lhitems, n, create_line(msg));
         itr = itr->next;
      }
      itr = p->lhbdep->head;
      while (itr != NULL) {
         prt = (Port *)itr->item;
         sprintf(msg, "Build depend: %-40.40s (%-.20s)",
               prt->name, (prt->state >= STATE_INSTALLED) ? "installed":
               "not installed");
         n = add_list_item_after(lhitems, n, create_line(msg));
         itr = itr->next;
      }
      itr = p->lhrdep->head;
      while (itr != NULL) {
         prt = (Port *)itr->item;
         sprintf(msg, "Run depend  : %-40.40s (%-.20s)",
               prt->name, (prt->state >= STATE_INSTALLED) ? "installed":
               "not installed");
         n = add_list_item_after(lhitems, n, create_line(msg));
         itr = itr->next;
      }
      sprintf(msg, "Homepage    : %-.60s", p->url);
      n = add_list_item_after(lhitems, n, create_line(msg));

      /* init plistfile */
      sprintf(plistfile, "%s/pkg-plist", p->path);
      lhplist = parse_plist(p, plistfile);
      if (lhplist != NULL) {
         itr = lhplist->head;
         while (itr != NULL) {
            Plist *pl = (Plist *)itr->item;
            sprintf(msg, "file of port: %-40.40s (%-.20s)",
                  pl->name, (pl->exist == TRUE) ? "installed" : "not installed");
            /* clean up */
            free(pl->name);
            free(pl);
            n = add_list_item_after(lhitems, n, create_line(msg));
            itr = itr->next;
         }
         free_list(lhplist);
      }

      if (browse_list(lhitems, p->name, FALSE) > 0)
         redraw_dimensions = TRUE;
      /* all lines will be freed by browse_list at the end,
         but the lhitems list still exist, so free it */
      free_list(lhitems);
   }
}

/* generic list browser, prev means parent item, if exists */
int
browse_list(Lhd *lh, void *parent, bool proceed) {
   extern WINDOW *wbrowse;
   extern State state;
   extern Lhd *lhprts;
   extern bool redraw_dimensions;
   void *items[lh->num_of_items];
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
   char msg[MAX_COLS];
   Lhd *lhitems;
   Category *cat = NULL;
   Point pt;
   bool search_highlight = FALSE;
   short type;

   if (lh->num_of_items < 1) /* return immediately, if there aren't
                                any items */
      return (-1); 

   /* initialization */
   create_array_from_list(lh, items); 
   wprint_cmdinfo("");
   doupdate();
   wclear(wbrowse);
   getmaxyx(wbrowse, maxy, maxx);
   /* Line is common minimum struct, this will work for all
      bigger structs, such as Option, Category, Port */
   type = ((Line *)items[0])->type;
   
   /* trick for categories with fewer than maxy items */
   if (lh->num_of_items < maxy) maxy = lh->num_of_items;
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
         if (lh->num_of_items < maxy) maxy = lh->num_of_items;
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
      if (type == CATEGORY) {
         set_cat_titlestatus(topidx, topidx + maxy);
      } else if (type == PORT) {
         /* here parent is needed */
         set_ports_titlestatus((Category *)parent, topidx,
               topidx + maxy, proceed);
      } else if (type == OPTION) {
         set_option_titlestatus((Port *)parent, topidx, topidx + maxy);
      } else if (type == LINE) {
         set_line_titlestatus((char *)parent, topidx, topidx + maxy,
               lh->num_of_items);
      }

      /* begin of redraw part */
      switch (redraw) {
         case REFRESH_WINDOW:
            for (i = 0; i < maxy; i++) {
               wprint_item(wbrowse, i, 0, items[topidx + i]);
               if ((curridx == i) && (type != LINE))
                  mvwchgat(wbrowse, curridx, 0, -1,
                        COLOR_PAIR(CLR_SELECTOR + 1), 0, NULL);
               else if (search_highlight) 
                  mvwchgat(wbrowse, curridx, pt.x, strlen(expstr),
                        COLOR_PAIR(CLR_BROWSE + 1) | A_REVERSE, 0, NULL);
            }
            break;
         case REFRESH_ENTRY:
            if (type != LINE) {
               /* only highlight if it's no line */
               wprint_item(wbrowse, previdx, 0, items[topidx + previdx]);
               mvwchgat(wbrowse, previdx, 0, -1,
                     COLOR_PAIR(CLR_BROWSE + 1), 0, NULL);
               wprint_item(wbrowse, curridx, 0, items[topidx + curridx]);
               mvwchgat(wbrowse, curridx, 0, -1,
                     COLOR_PAIR(CLR_SELECTOR + 1), 0, NULL);
            }
            break;
      }
      /* end of redraw */

      /* begin of key press handling */      
      previdx = curridx;
      press = wgetch(wbrowse);
      switch (press) {
         case 'j':
         case KEY_DOWN:
            if ((curridx < maxy - 1) && (type != LINE)) {
               curridx++;
               redraw = REFRESH_ENTRY;
            } else if ((topidx + maxy) < lh->num_of_items) {
               topidx++;
               redraw = REFRESH_WINDOW;
            }
            search_highlight = FALSE;
            break;
         case 'k':
         case KEY_UP:
            if ((curridx > 0) && (type != LINE)) {
               curridx--;
               redraw = REFRESH_ENTRY;
            } else if (topidx > 0) {
               topidx--;
               redraw = REFRESH_WINDOW;
            }
            search_highlight = FALSE;
            break;
         case 'g': /* top */
            topidx = curridx = 0;
            redraw = REFRESH_WINDOW;
            search_highlight = FALSE;
            break;
         case 'G': /* bottom */
            topidx = lh->num_of_items - maxy; 
            curridx = maxy - 1;   
            redraw = REFRESH_WINDOW;
            search_highlight = FALSE;
            break;
         case KEY_NPAGE: /* pg_down */
            if ((topidx + (2 * maxy)) < lh->num_of_items)
               topidx += maxy;  
            else {
               topidx = lh->num_of_items - maxy;
               curridx = maxy - 1;
            }
            redraw = REFRESH_WINDOW;
            search_highlight = FALSE;
            break;   
         case KEY_PPAGE: /* pg_up */
            if ((topidx - maxy) > 0)
               topidx -= maxy;
            else 
               topidx = curridx = 0;
            redraw = REFRESH_WINDOW;
            search_highlight = FALSE;
            break;
         case 'l':
         case KEY_RIGHT:
         case '\n': /* ENTER */
            if (((Category *)items[topidx + curridx])->type == CATEGORY) {
               if (browse_list(((Category *)items[topidx + curridx])->lhprts,
                        items[topidx + curridx], FALSE) > 0)
                  redraw_dimensions = TRUE;
               else
                  redraw = REFRESH_WINDOW;
            } else if (((Port *)items[topidx + curridx])->type == PORT) {
               Port *prt = (Port *)items[topidx + curridx];
               lhitems = parse_file(prt->pathpkgdesc);
               if (browse_list(lhitems, prt->pathpkgdesc, FALSE) > 0)
                  redraw_dimensions = TRUE;
               /* all lines will be freed by browse_list at the end,
                  but the lhitems still exist, so free it */
               free_list(lhitems);
               redraw = REFRESH_WINDOW;
            }
            break; 
         case KEY_F(1):
         case 'h': /* help */
            lhitems = parse_file(HELP_FILE);
            if (browse_list(lhitems, HELP_FILE, FALSE) > 0)
               redraw_dimensions = TRUE;
            /* all lines will be freed by browse_list at the end,
               but the lhitems list still exist, so free it */
            free_list(lhitems);
            redraw = REFRESH_WINDOW;
            search_highlight = FALSE;
            break;   
         case '/': /* fw search */
            expstr = wprint_inputoutput_str(" forward search key: ");
            pt = search(items, lh->num_of_items, expstr, topidx + curridx, 1);
            lastidx = nextidx = pt.y;
            if (nextidx != -1) { /* found */
               fw_search(lh->num_of_items, nextidx, maxy, &topidx, &curridx);
               redraw = REFRESH_WINDOW;
               search_highlight = TRUE;
            } else {
               wprint_cmdinfo(" no (more) items found in bottom direction");
               doupdate();
            }
            break;
         case 'n': /* fw search next */
            if (expstr != NULL) {
               wprint_cmdinfo("");
               doupdate();
               if (topidx + curridx != nextidx) {
                  pt = search(items, lh->num_of_items, expstr, topidx + curridx, 1);
                  nextidx = pt.y;
               } else {
                  if (nextidx + 1 < lh->num_of_items) {
                     pt = search(items, lh->num_of_items, expstr, nextidx + 1, 1);
                     nextidx = pt.y;
                  }
               }

               if ((nextidx != -1) && (lastidx != nextidx)) { /* found */
                  fw_search(lh->num_of_items, nextidx, maxy, &topidx, &curridx);
                  lastidx = nextidx;
                  redraw = REFRESH_WINDOW;
                  search_highlight = TRUE;
               } else {
                  wprint_cmdinfo(" no (more) items found in bottom direction");
                  doupdate();
               }
            }
            break;
         case '?': /* bw search */
            expstr = wprint_inputoutput_str(" backward search key: ");
            pt = search(items, lh->num_of_items, expstr, topidx + curridx, -1);
            lastidx = nextidx = pt.y;
            if (nextidx != -1) { /* found */
               bw_search(lh->num_of_items, nextidx, maxy, &topidx, &curridx);
               redraw = REFRESH_WINDOW;
               search_highlight = TRUE;
            } else {
               wprint_cmdinfo(" no (more) items found in top direction");
               doupdate();
            }
            break;
         case 'N': /* bw search next */
            if (expstr != NULL) {
               wprint_cmdinfo("");
               doupdate();
               if (topidx + curridx != nextidx) {
                  pt = search(items, lh->num_of_items, expstr, topidx + curridx, -1);
                  nextidx = pt.y;
               } else {
                  if (nextidx - 1 >= 0) {
                     pt = search(items, lh->num_of_items, expstr, nextidx - 1, -1);
                     nextidx = pt.y;
                  }
               }
               if ((nextidx != -1) && (lastidx != nextidx)) { /* found */
                  bw_search(lh->num_of_items, nextidx, maxy, &topidx, &curridx);
                  lastidx = nextidx;
                  redraw = REFRESH_WINDOW;
                  search_highlight = TRUE;
               } else {
                  wprint_cmdinfo(" no (more) items found in top direction");
                  doupdate();
               }
            }
            break;
         case 't': /* show terminal screen */
            /* leaving curses ... */
            def_prog_mode(); /* save current tty modes */
            endwin();        /* restore original tty modes */
            fpurge(stdin);
            getchar();
            /* ... coming back to curses */
            refresh();
            redraw_dimensions = TRUE;
            break;
      }

      /* special key handling, if not lines browsing */
      if (type != LINE) {
         switch(press) {
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
               if (type == PORT) 
                  lhitems = lh;
               else
                  lhitems = lhprts;
               if (expch == 's') {
                  int state;
                  expch = wprint_inputoutput_ch(" Which state? [</>/=/B/R/i/u/d] ");

                  switch (expch) {
                     case '<':
                        state = STATE_INSTALLED_OLDER;
                        cat = create_filter_category(lhitems,
                              "filter of older than installed port(s)", STATE, &state);
                        break;   
                     case '>':
                        state = STATE_INSTALLED_NEWER;
                        cat = create_filter_category(lhitems,
                              "filter of newer than installed port(s)", STATE, &state);
                        break;
                     case '=':
                        state = STATE_INSTALLED;   
                        cat = create_filter_category(lhitems,
                              "filter of installed port(s)", STATE, &state);
                        break;
                     case 'B':
                        state = STATE_BDEP;   
                        cat = create_filter_category(lhitems,
                              "filter of build dependencies", STATE, &state);
                        break;
                     case 'R':
                        state = STATE_RDEP;   
                        cat = create_filter_category(lhitems,
                              "filter of run dependencies", STATE, &state);
                        break;
                     case 'i':
                        state = STATE_INSTALL;   
                        cat = create_filter_category(lhitems,
                              "filter of marked ports for installation", STATE, &state);
                        break;
                     case 'u':
                        state = STATE_UPDATE;   
                        cat = create_filter_category(lhitems,
                              "filter of marked ports for update", STATE, &state);
                        break;
                     case 'd':
                        state = STATE_DEINSTALL;   
                        cat = create_filter_category(lhitems,
                              "filter of marked ports for deinstallation", STATE, &state);
                        break;
                     default: /* error, don't know */
                        i = -1;
                        break;
                  }
               } else if (expch == 'k') {
                  expstr = wprint_inputoutput_str(" keyword: ");
                  sprintf(msg, "filter of ports with keyword '%-.20s'", expstr);
                  cat = create_filter_category(lhitems, msg, STRING, expstr);
               } else
                  i = -1;
               if (i != 0) {
                  wprint_cmdinfo(" No valid filter");
                  doupdate();
               } else { /* everything valid */
                  i = browse_list(cat->lhprts, cat, FALSE);
                  if (i < 0) 
                     wprint_cmdinfo(" There aren't any matching ports");
                  else if (i > 0)
                     redraw_dimensions = TRUE;
                  else
                     wprint_cmdinfo("");
                  doupdate();

                  free(cat->name);
                  free_list(cat->lhprts);
                  free(cat);
                  redraw = REFRESH_WINDOW;
               }
               break;
         }
      } else { /* line browsing */
         if (press == KEY_LEFT)
            press = 'q';
      } 

      /* special key press handling for ports browsing */
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
               if (p->state >= STATE_INSTALLED) {
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
                     if (p->lhopts == NULL)
                        create_options(p);
                     if (browse_list(p->lhopts, p, FALSE) > 0)
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
                     mark_ports(lh, STATE_INSTALL);
                     redraw = REFRESH_WINDOW;
                     break;
                  case 'u':
                     mark_ports(lh, STATE_UPDATE);
                     redraw = REFRESH_WINDOW;
                     break;
                  case 'd':
                     mark_ports(lh, STATE_DEINSTALL);
                     redraw = REFRESH_WINDOW;
                     break;
                  case ' ':
                     mark_ports(lh, STATE_NOT_SELECTED);
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

   if (type == LINE) {
      /* free up lines */
      for (i = 0; i < lh->num_of_items; i++) 
         free_line((Line *)items[i]);
   }

   wclear(wbrowse);
   return (result);
}

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

/* returns input char */
int
wprint_inputoutput_ch(char *output) {
   extern WINDOW *wcmd;
   int input;

   mvwprintw(wcmd, 0, 0, output);
   wclrtoeol(wcmd);
   echo();
   input = wgetch(wcmd);
   noecho();

   return input;
}

/* returns input string */
char *
wprint_inputoutput_str(char *output) {
   extern WINDOW *wcmd;
   char input[20];

   mvwprintw(wcmd, 0, 0, output);
   wclrtoeol(wcmd);
   echo();
   mvwgetnstr(wcmd, 0, strlen(output), input, 15);
   noecho();

   return strdup(input);
}

void
wprint_cmdinfo(char *msg) {
   extern WINDOW *wcmd;

   mvwprintw(wcmd, 0, 0, msg);
   wclrtoeol(wcmd);
   wnoutrefresh(wcmd);
}

void
wprint_titlebar(char *s, bool help) {
   extern WINDOW *wtop;

   mvwprintw(wtop, 0, 0, s);
   wclrtoeol(wtop);
   if (help) {
      mvwprintw(wtop, 0, (wtop->_maxx - 10), "(h)elp");
      wclrtoeol(wtop);
   }
   mvwchgat(wtop, 0, 0, -1, COLOR_PAIR(CLR_TITLE + 1), 0, NULL);
   wnoutrefresh(wtop);
}

void
wprint_statusbar(char *s) {
   extern WINDOW *wbottom;
   mvwprintw(wbottom, 0, 0, s);
   wclrtoeol(wbottom);
   mvwchgat(wbottom, 0, 0, -1, COLOR_PAIR(CLR_STATUS + 1), 0, NULL);
   wnoutrefresh(wbottom);
}

/* prints items of list browser */
void
wprint_item(WINDOW *w, int y, int x, void *item) {
   char itemstr[MAX_COLS];
         
   if (((Category *)item)->type == CATEGORY) { /* print category */
/* resize sigsev hochrunter, hmm */

      Category *cat = (Category *)item;
      sprintf(itemstr, " [ ] %-20.20s\t-%3d/ +%3d/%5d/%5d port(s)",
            cat->name, cat->num_of_deinst_ports,
            cat->num_of_marked_ports,
            cat->num_of_inst_ports,
            cat->num_of_ports);
      if ((cat->num_of_marked_ports != 0) &&
            (cat->num_of_deinst_ports != 0))
         itemstr[2] = '*';
      else if (cat->num_of_marked_ports != 0)
         itemstr[2] = '+';
      else if (cat->num_of_deinst_ports != 0)
         itemstr[2] = '-';

   } else if (((Port *)item)->type == PORT) { /* print port */
      Port *p = (Port *)item;
      sprintf(itemstr, " [ ] %-20.20s\t%-.47s", p->name, p->descr);
      switch (p->state) {
         case STATE_INSTALL:
            itemstr[2] = 'i'; /* install */
            break;
         case STATE_UPDATE:
            itemstr[2] = 'u'; /* upgrade */
            break;
         case STATE_DEINSTALL:
            itemstr[2] = 'd'; /* delete */
            break;
         case STATE_BDEP:
            itemstr[2] = 'B'; /* build dependency */
            break;
         case STATE_RDEP:
            itemstr[2] = 'R'; /* run dependency */
            break;
         case STATE_INSTALLED:
            itemstr[2] = '='; 
            break;
         case STATE_INSTALLED_OLDER:
            itemstr[2] = '<';
            break;
         case STATE_INSTALLED_NEWER:
            itemstr[2] = '>';
            break;
      }
   } else if (((Option *)item)->type == OPTION) { /* compile option */
      Option *opt = (Option *)item;
      sprintf(itemstr, "  < > %-.70s", opt->name);
      if (opt->state == STATE_SELECTED) itemstr[3] = 'X';
   } else if (((Line *)item)->type == LINE) { /* simple line */
      sprintf(itemstr, "%-.79s", ((Line *)item)->name);
   }

   mvwprintw(w, y, x, itemstr);
   wclrtoeol(w);
}
 

/* handles resizes, if xterm was resized */
void
resizeHandler(int sig) {
   extern WINDOW *wtop;
   extern WINDOW *wbottom;
   extern WINDOW *wbrowse;
   extern WINDOW *wcmd;
   extern bool redraw_dimensions;
   int maxy, maxx;
   struct winsize ws;

   /* getmaxyx don't works quiet well */
   ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
   maxy = ws.ws_row;
   maxx = ws.ws_col;

   /* resize terminal */
   resizeterm(maxy, maxx);
   
   clearok(wtop, TRUE);
   wresize(wtop, 1, maxx);
   wnoutrefresh(wtop);

   clearok(wbrowse, TRUE);
   wresize(wbrowse, maxy - 3, maxx);
   wnoutrefresh(wbrowse);

   clearok(wbottom, TRUE);
   wresize(wbottom, 1, maxx);
   wnoutrefresh(wbottom);
   mvwin(wbottom, maxy - 2, 0);
   wnoutrefresh(wbottom);

   clearok(wcmd, TRUE);
   wresize(wcmd, 1, maxx);
   wnoutrefresh(wcmd);
   mvwin(wcmd, maxy - 1, 0);
   wnoutrefresh(wcmd);

   doupdate();

   /* notifies active browser to rearrange after next
      user interaction */
   redraw_dimensions = TRUE;
}

/* initializes all windows and returns the browser window */
void
init_windows() {
   extern WINDOW *wtop;
   extern WINDOW *wbottom;
   extern WINDOW *wbrowse;
   extern WINDOW *wcmd;
   int maxy, maxx;

   /* init */
   maxy = LINES;
   maxx = COLS;

   /* title bar */
   wtop = newwin(1, maxx, 0, 0);
   wbkgd(wtop, COLOR_PAIR(CLR_TITLE + 1));

   /* status bar */
   wbottom = newwin(1, maxx, maxy - 2, 0);
   wbkgd(wbottom, COLOR_PAIR(CLR_STATUS + 1));
   
   /* browser window */
   wbrowse = newwin(maxy - 3, maxx, 1, 0);
   wbkgd(wbrowse, COLOR_PAIR(CLR_BROWSE + 1));

   /* cmd window */
   wcmd = newwin(1, maxx, maxy - 1, 0);
   wbkgd(wcmd, COLOR_PAIR(CLR_CMD + 1));
}

void
clean_up_windows() {
   extern WINDOW *wtop;
   extern WINDOW *wbottom;
   extern WINDOW *wbrowse;
   extern WINDOW *wcmd;

   /* clean up */
   delwin(wbrowse);
   delwin(wbottom);
   delwin(wtop);
   delwin(wcmd);
}

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
error(char *s) {
   printf("\n%s\nPress any key to continue\n", s);
   getchar();
}

void
proceed_action(Lhd *lh) {
   extern Lhd *lhcats;
   extern State state;
   extern Config config;
   extern bool redraw_dimensions;
   Iter itr = lh->head;
   Iter citr; /* cat iterator */
   Iter oitr; /* option iterator */
   Iter ditr; /* dependency iterator */
   Port *p;
   char execstr[1000];
   char option[MAX_TOKEN];
   char wdir[MAX_PATH];
   int result = 0;

   /* leaving curses ... */
   def_prog_mode(); /* save current tty modes */
   endwin();        /* restore original tty modes */

   getcwd(wdir, MAX_PATH);
   /* action loop */
   while ((itr != NULL) && (result == 0)) {
      if (((Port *)itr->item)->type == PORT) {
         p = (Port *)itr->item;
         if (p->lhopts != NULL)
            oitr = p->lhopts->head; 

         switch (p->state) {
            case STATE_INSTALL:
               sprintf(execstr, "%s %s", config.make_cmd,
                     config.make_target[MK_TARGET_INST]);
              break;
            case STATE_UPDATE:
               sprintf(execstr, "%s %s", config.make_cmd,
                     config.make_target[MK_TARGET_UPDATE]);
               break;   
            case STATE_DEINSTALL:
               sprintf(execstr, "%s %s", config.make_cmd,
                     config.make_target[MK_TARGET_DEINST]);
               break;   
         }

         /* cat compile options */
         while (oitr != NULL) {
            Option *opt = (Option *)oitr->item;
            if (opt->state == STATE_SELECTED) {
               sprintf(option, " %s", opt->arg);
               strcat(execstr, option);
            }
            oitr = oitr->next;
         }

         /* fire up make */
         chdir(p->path);
         result = system(execstr);
         switch (result) {
            case -1:
               error("fork() could not be invoked");
               return;
               break;
            case 127:
               error("execution of shell failed");
               return;
               break;
            case 0: /* build/installation/deinstallation successful */
               switch (p->state) {
                  case STATE_INSTALL:
                  case STATE_UPDATE:
                     p->state = STATE_INSTALLED;
                     ditr = p->lhbdep->head;
                     while (ditr != NULL) {
                        if (((Port *)ditr->item)->state != STATE_INSTALLED) 
                           ((Port *)ditr->item)->state = STATE_INSTALLED;
                        ditr = ditr->next;
                     }
                     break;
                  case STATE_DEINSTALL:
                     p->state = STATE_NOT_SELECTED;
                     break;
               }
               break;
         }
      }
      itr = itr->next;
   }
   chdir(wdir);
   refresh_cat_states(); 

   /* ... coming back to curses */
   refresh();
   redraw_dimensions = TRUE;

}

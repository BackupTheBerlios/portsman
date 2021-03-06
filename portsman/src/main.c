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

/* prints version to stdout */
void
version() {
   fprintf(stdout, "This is %s,\n\t(C)opyright 2002, 2003 by"
        " Anselm R. Garbe\n", _VERSION);
}

/* prints usage of portsman to stderr */
void
usage() {
   fprintf(stderr, "usage: portsman [-c <portsmanrc file>] [-h] [-v] [-P] [-s]\n" 
          "                [-r <rsync hostname>] [-d <ports dir> |\n"
          "                 -i <INDEX file>] [-p <PKG dir>]\n\n" 
          "       -c      path to user defined configfile\n"
          "       -h      prints this help\n" 
          "       -v      prints out the version of portsman\n"
          "       -P      uses only physically existing categories, no\n"
          "               meta categories (except \"All\" and \"Installed\")\n"
          "       -s      defines, that portsman automatically synchronizes\n"
          "               your INDEX file to current state (using rsync)\n"
          "       -r      defines additional rsync hostname which is used to\n"
          "               synchronize your used INDEX file\n" 
#if defined(__FreeBSD__) || defined(__OpenBSD__)
          "       -d      path of ports collection\n" 
#elif defined(__NetBSD__)
          "       -d      path of pkgsrc collection\n"
#endif
          "       -i      path to the ports collection INDEX file\n" 
          "               (default: %s)\n"
          "       -p      path to package database dir\n" 
          "               (default: %s)\n\n",
         INDEX_FILE, INSTALLED_PKG_DIR); 
}

/* init rsync server config */
void init_rsynchosts() {
   extern Config config;
   config.lrsynchosts = (List *)malloc(sizeof(List));
   config.lrsynchosts->head = config.lrsynchosts->tail = NULL;
   config.lrsynchosts->num_of_items = 0;

   add_list_item(config.lrsynchosts, RSYNC_SERVER_1);
   add_list_item(config.lrsynchosts, RSYNC_SERVER_2);
}

/* main function of portsman */
int
main(int argc, char * argv[]) {
   extern List *lcats;
   extern List *lphycats;
   extern List *lprts;
   extern Config config;
   extern bool redraw_dimensions;
   extern char *optarg;
   char *home_dir;
   char *ports_dir;
   char *index_file;
   char *inst_pkg_dir;
   char *config_file;
   char path[MAX_PATH];
   int c = 0;
   int result;
   bool autosync = FALSE;
 
   /* first of all check root permissions */
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
   if (getuid() != 0) {
      fprintf(stderr, "error: This utility should only be run as root.\n");
      exit(1);
   }
#endif

	/* pre init */
   home_dir = getenv("HOME");
   if (home_dir == NULL) {
      fprintf(stderr, "error: Can't determine your $HOME directory.\n");
      config_file = NULL;
   } else {
      sprintf(path, "%s/%s", getenv("HOME"), CONFIG_FILE);
      config_file = path;
   }

   index_file = INDEX_FILE;
   inst_pkg_dir = INSTALLED_PKG_DIR;
   ports_dir = PORTS_DIR;
   config.use_metacats = TRUE;
   config.fcolors[CLR_TITLE] = COLOR_BLACK;
   config.bcolors[CLR_TITLE] = COLOR_CYAN;
   config.fcolors[CLR_BROWSE] = COLOR_WHITE;
   config.bcolors[CLR_BROWSE] = COLOR_BLUE;
   config.fcolors[CLR_SELECTOR] = COLOR_BLUE;
   config.bcolors[CLR_SELECTOR] = COLOR_WHITE;
   config.fcolors[CLR_STATUS] = COLOR_BLACK;
   config.bcolors[CLR_STATUS] = COLOR_CYAN;
   config.fcolors[CLR_CMD] = COLOR_WHITE;
   config.bcolors[CLR_CMD] = COLOR_BLUE;
   config.make_cmd = "make";
   config.make_target[MK_TARGET_INST] = "install";
   config.make_target[MK_TARGET_DEINST] = "deinstall";
   config.make_target[MK_TARGET_UPDATE] = "reinstall";
   config.make_option[MK_OPTION_FORCE] = STATE_NOT_SELECTED;
   config.make_option[MK_OPTION_PKG] = STATE_NOT_SELECTED;
   config.make_option[MK_OPTION_CLEAN] = STATE_SELECTED;
   config.make_option[MK_OPTION_NOCHKSUM] = STATE_NOT_SELECTED;
   config.make_option[MK_OPTION_NODEPS] = STATE_NOT_SELECTED;
   config.make_option[MK_OPTION_FORCEPKGREG] = STATE_NOT_SELECTED;
   config.make_option[MK_OPTION_NOPKGREG] = STATE_NOT_SELECTED;
   config.make_option_arg[MK_OPTION_FORCE] = "-k";
   config.make_option_arg[MK_OPTION_PKG] = "package";
   config.make_option_arg[MK_OPTION_CLEAN] = "clean";
   config.make_option_arg[MK_OPTION_NOCHKSUM] = "NO_CHECKSUM=yes";
   config.make_option_arg[MK_OPTION_NODEPS] = "NO_DEPENDS=yes";
   config.make_option_arg[MK_OPTION_FORCEPKGREG] = "FORCE_PKG_REGISTER=yes";
   config.make_option_arg[MK_OPTION_NOPKGREG] = "NO_PKG_REGISTER=yes";
   init_rsynchosts();
   config.rsync_cmd = "rsync -uvz";

   /* command line args */
   while ((c = getopt(argc, argv, "vhPsr:d:i:p:c:")) != -1)
      switch(c) {
         case 'v':
            version();
            exit(0);
            break;
         case 'P':
            config.use_metacats = FALSE;
            break;
         case 's':
            autosync = TRUE;
            break;
         case 'r':
            add_list_item(config.lrsynchosts, strdup(optarg));
            break;
         case 'd':
            ports_dir = strdup(optarg);
            break;
         case 'i': /* INDEX file */
            index_file = strdup(optarg);
            break;
         case 'p': /* pkg db dir */
            inst_pkg_dir = strdup(optarg);
            break;
         case 'c': /* config file */
            config_file = strdup(optarg);
            break;
         case '?': /* not known */
         case 'h':
            usage();
            exit(1);
            break;
      }

   /* parse rc config file */
   if (config_file != NULL) {
      if ((result = parse_rc_file(config_file)) != 0) {
         if (result != ERROR_NO_RC_FILE)
            fprintf(stderr, "error: portsmanrc configuration file corrupted"
               " in line %d\n", result);
      }
   }
 
   config.index_file = index_file;
   config.inst_pkg_dir = inst_pkg_dir;
   config.ports_dir = ports_dir;

   fprintf(stdout, "Checking state of %s...\n", config.index_file);
   if (!is_index_uptodate(config.ports_dir, TRUE)) {
      /* download, start user interaction etc. */
      fprintf(stdout, "%s is not as up to date as your ports collection:\n",
            config.index_file);
      if (autosync) {
         fprintf(stdout, "Auto synchronizing %s...\n", config.index_file);
         c = 's';
      } else {
         fprintf(stdout, "Choose (s)ynchronize, (m)ake index or (i)gnore: "); 
         fflush(stdin);
         c = getc(stdin);
      }
      if (c == 's')
         sync_index();
      else if (c == 'm')
         make_index();
   }
 
   /* init */
   lcats = (List *)malloc(sizeof(List));
   lcats->head = NULL;
   lcats->tail = NULL;
   lcats->num_of_items = 0;
   lphycats = (List *)malloc(sizeof(List));
   lphycats->head = NULL;
   lphycats->tail = NULL;
   lphycats->num_of_items = 0;
   lprts = (List *)malloc(sizeof(List));
   lprts->head = NULL;
   lprts->tail = NULL;
   lprts->num_of_items = 0;
   redraw_dimensions = FALSE;

	fprintf(stdout, "Please stand by while portsman is coming up...\n");
   /* create data structure */
   result = parse_index();
   if (result == ERROR_OPEN_INDEX) {
      fprintf(stderr, "error: Can't open INDEX file: %s\n", config.index_file);
      exit(1);
   } else if (result == ERROR_CORRUPT_INDEX) {
      fprintf(stderr, "error: INDEX file seems to be corrupted or is of "
            "an unknown format.\n");
      exit(1);
   }
   /* resize signal handler */
   signal(SIGWINCH, resizeHandler);

   /* initializes curses */
   initscr();
   start_color();

   if(has_colors () == TRUE) 
   {
      /* Note: + 1 in each pair is needed, to don't
         maipulate COLOR_PAIR(0) */
      init_pair(CLR_TITLE + 1, config.fcolors[CLR_TITLE],
            config.bcolors[CLR_TITLE]);
      init_pair(CLR_BROWSE + 1, config.fcolors[CLR_BROWSE],
            config.bcolors[CLR_BROWSE]);
      init_pair(CLR_SELECTOR + 1, config.fcolors[CLR_SELECTOR],
            config.bcolors[CLR_SELECTOR]);
      init_pair(CLR_STATUS + 1, config.fcolors[CLR_STATUS],
            config.bcolors[CLR_STATUS]);
      init_pair(CLR_CMD + 1, config.fcolors[CLR_CMD],
            config.bcolors[CLR_CMD]);
   }

   cbreak();

   /* initializes windows */
   init_windows();

   /* open browser */
   do {
      if (config.use_metacats)
         result = browse_list(lcats, lcats->head->item, FALSE, FALSE);
      else
         result = browse_list(lphycats, lphycats->head->item, FALSE, FALSE);
   } while (result == CATS_TOGGLED);

   /* clean up all windows */
   clean_up_windows();

   endwin();
   /* curses deinitialized */

   return (0);
}

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

/* prints version to stdout */
void
version() {
   fprintf(stdout, "This is %s,\n\t(C)opyright 2002, 2003 by Anselm R. Garbe\n", _VERSION);
}

/* prints usage of portsman to stderr */
void
usage() {
   fprintf(stderr, "usage: portsman [-c <portsmanrc file>] [-h] [-v]\n" 
          "                [-i <INDEX file>] [-p <PKG dir>]\n\n" 
          "       -c      path to user defined configfile\n"
          "       -h      prints this help\n" 
          "       -v      prints out the version of portsman\n"
          "       -i      path to the ports collection INDEX file\n" 
          "               (default: %s)\n"
          "       -p      path to package database dir\n" 
          "               (default: %s)\n\n",
         INDEX_FILE, INSTALLED_PKG_DIR); 
}

/* main function of portsman */
int
main(int argc, char * argv[]) {
   extern Lhd *lhcats;
   extern Lhd *lhprts;
   extern State state;
   extern Config config;
   extern bool redraw_dimensions;
   extern char *optarg;
   char *home_dir;
   char *index_file;
   char *inst_pkg_dir;
   char *config_file;
   char path[MAX_PATH];
   int c = 0;
   int result;
 
   /* first of all check root permissions, only under FreeBSD */
#if defined(__FREEBSD__)
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

   /* command line args */
   while ((c = getopt(argc, argv, "vi:p:c:")) != -1)
      switch(c) {
         case 'v':
            version();
            exit(0);
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
            usage();
            exit(1);
            break;
      }

   /* parse rc config file */
   if (config_file != NULL) {
      if (parse_rc_file(config_file) == ERROR_CORRUPT_RC_FILE) {
         fprintf(stderr, "error: portsmanrc configuration file corrupted\n");
      }
   }
 
   config.index_file = index_file;
   config.inst_pkg_dir = inst_pkg_dir;

   /* init */
   lhcats = (Lhd *)malloc(sizeof(Lhd));
   lhcats->head = NULL;
   lhcats->num_of_items = 0;
   lhprts = (Lhd *)malloc(sizeof(Lhd));
   lhprts->head = NULL;
   lhprts->num_of_items = 0;
	state.num_of_inst_ports = 0;
   state.num_of_ports = 0;
   state.num_of_cats = 0;
   state.num_of_marked_ports = 0;
   state.num_of_deinst_ports = 0;
   redraw_dimensions = FALSE;

	fprintf(stdout, "Please stand by while portsman is coming up...\n");
   /* create data structure */
   result = parse_index();
   if (result == ERROR_OPEN_INDEX) {
      fprintf(stderr, "error: Can't open INDEX file: %s\n", config.index_file);
      exit(1);
   } else if (result == ERROR_OPEN_PDB_DIR) {
      fprintf(stderr, "error: Can't open pkg database dir: %s\n", config.inst_pkg_dir);
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
   browse_list(lhcats, NULL, FALSE);

   /* clean up all windows */
   clean_up_windows();

   endwin();
   /* curses deinitialized */

   return (0);
}

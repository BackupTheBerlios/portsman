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
#include "list.h"
#include "tree.h"

/* type for entries */
typedef struct port_t {
   short type; /* provide runtime check for void pointer */
   char *name; /* prog-version */
   char *path; /* path of port */
   char *instpfx; /* install prefix */
   char *descr; /* descr. of port */
   char *pathpkgdesc; /* path to pkg-descr */
   char *maintainer; /* name & email of maintainer */
   char *url; /* homepage of port */
   short state; /* see STATE_* in consts.h for detail */
   struct list_t *lcats; /* list of category pointers */
   struct list_t *lopts; /* list of port options */
   struct list_t *lbdep; /* list of runtime deps */
   struct list_t *lrdep; /* list of build deps */
   struct list_t *ldep; /* list of ports, which have this port
                           ad dependency */
} Port;

/* type for port categories */
typedef struct category_t {
   short type; /* provide runtime check for void pointer */
   char *name;
   bool meta; /* differentiation of physical and meta categories */
   int num_of_ports;
   int num_of_inst_ports;
   int num_of_marked_ports;
   int num_of_deinst_ports;
   struct list_t *lprts;
} Category;

/* type for ports compile options */
typedef struct option_t {
   short type;
   char *name;
   char *arg;
   short state;
} Option;

/* simple line type for list browser to supply
   browsing summary, files, etc. VERY COOL TRICK */
typedef struct line_t {
   short type;
   char *name;
} Line;

/* plist state type */
typedef struct plist_t {
   char *name;
   bool exist;
} Plist;

/* portsman configuration type */
typedef struct config_t {
   short fcolors[5]; /* foreground colors */
   short bcolors[5]; /* background colors */
   bool use_metacats;
   char *index_file;
   char *inst_pkg_dir;
   char *ports_dir;
   char *make_cmd;
   char *make_target[3];
   short make_option[7];
   char *make_option_arg[7];
} Config;

/* simple (y, x) point, needed for search key highlightning */
typedef struct point_t {
   int y;
   int x;
} Point;

/*
Copyright (c) 2002, 2003, Anselm R. Garbe
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of the portsman developers nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
   struct lhd_t *lhcats; /* list of category pointers */
   struct lhd_t *lhopts; /* list of port options */
   struct lhd_t *lhbdep; /* list of runtime deps */
   struct lhd_t *lhrdep; /* list of build deps */
} Port;

/* type for port categories */
typedef struct category_t {
   short type; /* provide runtime check for void pointer */
   char *name;
   int num_of_ports;
   int num_of_inst_ports;
   int num_of_marked_ports;
   int num_of_deinst_ports;
   struct lhd_t *lhports;
} Category;

/* type for ports compile options */
typedef struct option_t {
   short type;
   char *name;
   char *cmd;
   short state;
} Option;

/* portsman state type */
typedef struct state_t {
   int num_of_deinst_ports;
   int num_of_inst_ports;
   int num_of_ports;
   int num_of_cats;
   int num_of_marked_ports;
} State;

/* plist state type */
typedef struct plist_t {
   char *name;
   bool exist;
} Plist;

/* portsman configuration type */
typedef struct config_t {
   char *index_file;
   char *inst_pkg_dir;
   short fcolors[5]; /* foreground colors */
   short bcolors[5]; /* background colors */
} Config;

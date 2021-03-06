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
#ifndef _PORTSMAN_FNS_H
#define _PORTSMAN_FNS_H

/* compare.c */
char *
str_str(char *big, char *little); 
int
cmp_name(const void *item1, const void *item2); 
int
cmp_str(const void *item1, const void *item2); 
 
/* tree.c */
void
free_tree(TNode *t); 
void
create_inorder_list(List *l, TNode *t); 
TNode *
add_tree_item(TNode *t, void *item,
      int (*comp)(const void *, const void *));

/* list.c */
bool
has_item(List *l, void *item, 
      int (*comp)(const void *, const void *));
void
free_list(List *l);
Node *
add_list_item(List *l, void *item);
void
create_array_from_list(List *l, void *items[]);

/* parse.c */
int
parse_index();
Category *
add_category(char *name, List *lpdir); 
List *
parse_tokenlist(char *toklist, char *delim);
TNode *
parse_dir(char *path); 
List *
parse_options(char *mkfile); 
int
parse_rc_file(char *filepath); 
List *
parse_plist(Port *p, char *plistfile); 
List *
parse_file(char *filepath); 
 
/* manage.c */
List *
get_online_help();
void
free_port(Port *p); 
Port *
create_port(char *name, TNode *t); 
Line *
create_line(char *name); 
void
free_line(Line *l); 
void
mark_ports(List *l, int state); 
Category *
create_filter_category(List *lfilter, char *name,
      int type, void *item); 
Category *
create_proceed_category(); 
void
create_options(Port *p); 
Point
search(void *items[], int num_of_items, char *s, int start, int direction); 
void
unmark_all_dependencies(); 
void
mark_dependencies(Port *p); 
void
mark_port(Port *p, int state, int incrementor); 
void
refresh_cat_states(); 
void
refresh_cat_state(Category *cat); 
 
/* window.c */
void
resizeHandler(int sig);
int
wprint_inputoutput_ch(char *output);
char *
wprint_inputoutput_str(char *output);
void
wprint_cmdinfo(char *msg); 
void
wprint_item(WINDOW *w, int y, int x, void *item); 
void
wprint_statusbar(char *s); 
void
wprint_titlebar(char *s, bool help); 
void
init_windows();
void
clean_up_windows();

/* io.c */
void
debug(char *s);

/* browse.c */
int
browse_list(List *l, void *parent, bool proceed, bool artificia); 

/* system.c */
void
sync_index(); 
void
make_index(); 
bool
is_index_uptodate(char *path, bool top); 
void
proceed_action(List *l); 

/* main.c */
int
main(int argc, char *argv[]);
void
version(); 
void
usage(); 
#endif

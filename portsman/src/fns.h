/*
Copyright (c) 2002, 2003, Anselm R. Garbe
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of the portsman developers nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* compare.c */
char *
str_str(char *big, char *little); 
int
cmp_name(const void *item1, const void *item2); 
int
cmp_str(const void *item1, const void *item2); 
 
/* tree.c */
void
free_tree(TNode *troot); 
void
create_inorder_list(List *l, TNode *t); 
TNode *
add_tree_item(TNode *troot, void *item,
      int (*comp)(const void *, const void *));

/* list.c */
void
free_list(List *l);
Node *
add_list_item_after(List *l, Node *prev, void *item); 
Node *
add_list_item(List *head, void *item);
void
create_array_from_list(List *l, void *items[]);

/* parse.c */
int
parse_index();
Category *
add_category(char *name); 
List *
parse_tokenlist(char *toklist, char *delim);
TNode *
parse_dir(char *path); 
List *
parse_options(char *mkfile); 
int
parse_rc_file(char *filepath); 
List *
parse_plist(Port *port, char *plistfile); 
 
/* manage.c */
void
mark_ports(List *l, int state); 
Category *
create_filter_category(List *lfilter, char *name,
      int type, void *item); 
Category *
create_proceed_category(); 
void
create_options(Port *port); 
int
search(void *items[], int num_of_items, char *s, int start, int direction); 
void
unmark_all_dependencies(); 
void
mark_dependencies(Port *port); 
void
mark_port(Port *port, int state, int incrementor); 

/* window.c */
void
resizeHandler(int sig);
int
wprint_inputoutput_ch(char *output);
char *
wprint_inputoutput_str(char *output);
void
wprint_cmdinfo(char *msg); 
int
wprint_line(WINDOW *w, int *y, int x, char *s, bool quit); 
void
wprint_item(WINDOW *w, int y, int x, void *item); 
void
wprint_statusbar(char *s); 
void
wprint_titlebar(char *s);
void
init_windows();
void
clean_up_windows();

/* io.c */
void
debug(char *s);

/* browse.c */
void
set_file_titlebar(char *name); 
void
set_cat_titlestatus(int top, int bottom); 
void
set_ports_titlestatus(Category *cat, int top, int bottom,
      bool proceed); 
void
browse_port_summary(Port *port); 
int
browse_file(char *path);
int
browse_list(List *l, void *parent, bool proceed); 

/* system.c */
void
proceed_action(List *l); 

/* main.c */
int
main(int argc, char *argv[]);
void
version(); 
void
usage(); 

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

/* frees a list of items */
void free_list(List *l) {
   Node *n = l->head;
   Node *p = NULL;

   while (n != NULL) {
      p = n;
      n = n->next;
      free(p);
   }
}

/* creates items array from list l,
   Note: l has to be allocated and initialized */
void create_array_from_list(List *l, void *items[]) {
   Node *n = l->head;
   int i;

   for (i = 0; i < l->num_of_items; i++) {
      items[i] = n->item;
      n = n->next;
   }
}

/* adds an item to list l after prev node,
   Note: l has to be allocated and initialized  */
Node *add_list_item_after(List *l, Node *prev, void *item) {
   Node *new = (Node *)malloc(sizeof(Node));
   new->item = item;
   new->next = NULL;
   (l->num_of_items)++; 
 
   if ((l->head == NULL) || (prev == NULL)) { /* fresh list */
      l->head = new;
   } else {
     prev->next = new;
   }

   return new;
}

/* adds an item to the double linked list of list pointers head node,
   returns the new node, this function is more efficient than
   ordered_add_item, because it does not use comparisions */
Node *add_list_item(List *l, void *item) {
   Node *n = NULL;
   Node *p = NULL;

   /* if l == NULL then it's a new list */
   if (l->head != NULL) {
      n = l->head;
   }
   while (n != NULL) {
      p = n;
      n = n->next;
   }
   return add_list_item_after(l, p, item);
}

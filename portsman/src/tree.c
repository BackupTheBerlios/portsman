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

/* frees a (sub)tree */
void free_tree(TNode *troot) {
   TNode *t = troot;

   if (t == NULL) return;
   free_tree(t->left);
   free_tree(t->right);
   free(t);
}

/* creates inorder list l of (sub)tree t,
   Note: List l has to be allocated and initialized */
void create_inorder_list(List *l, TNode *t) {
   static Node *prev;

   if (t == NULL) return;
   /* left child */
   create_inorder_list(l, t->left);
   /* new list item */
   prev = add_list_item_after(l, prev, t->item);
   /* right child */
   create_inorder_list(l, t->right);
   /* clean up */
   free(t);
}

/* adds an item to the tree associated by troot, returns troot item,
   Note: If you're going to create a new tree, troot has to be NULL */
TNode *add_tree_item(TNode *troot, void *item,
      int (*comp)(const void *, const void *)) {
   TNode *t = troot;
   int cmp = 1;
   extern void *exists;

   if (t == NULL) { /* insert new leaf */
      t = (TNode *)malloc(sizeof(TNode));
      t->item = item;
      t->left = t->right = NULL; /* leaf, no childs */
   } else { /* recursion */
      if((cmp = (*comp)(t->item, item)) > 0) {
         /* insert as left leaf */
         t->left = add_tree_item(t->left, item, comp);
      } else if (cmp < 0) {
         /* insert as right leaf */
         t->right = add_tree_item(t->right, item, comp);
      } else {
         exists = t;
      }
   }

   return t;
}

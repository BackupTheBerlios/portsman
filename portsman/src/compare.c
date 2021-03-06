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

/* wrapper for strstr of string.h */
char *
str_str(char *big, char *little) {
   int biglen = strlen(big);
   int littlelen = strlen(little);

   if ((biglen < 1) || (littlelen < 1))
      return NULL;

   return strstr((biglen >= littlelen) ? big : little,
         (biglen < littlelen) ? big : little);
}

/* compares two items by its name */
int
cmp_name(const void *item1, const void *item2) {

	if (((Category *)item1)->type == CATEGORY) /* category */
		return strcmp(((Category *)item1)->name, ((Category *)item2)->name);
   else if (((Port *)item1)->type == PORT) /* port */
		return strcmp(((Port *)item1)->name, ((Port *)item2)->name);
   else if(((Option *)item1)->type == OPTION)
		return strcmp(((Option *)item1)->name, ((Option *)item2)->name);
   else if(((Line *)item1)->type == LINE)
		return strcmp(((Line *)item1)->name, ((Line *)item2)->name);
  else
      return -1000;
}

/* compares two char items */
int
cmp_str(const void *item1, const void *item2) {
   return strcmp((char *)item1, (char *)item2);
}

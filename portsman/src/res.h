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

/* this file contains glocal string resources */

const char *online_help[] =
{
   "",
" CONTROLS:",
"", 
"     [->|RETURN|l]     moves into ports respectively file browser", 
"", 
"     [<-|BACKSPACE]    moves back to ports respectively file browser", 
"",      
"     [KEY_HIGH|k]      moves to previous item in top direction", 
"", 
"     [KEY_DOWN|j]      moves to next item in bottom direction", 
"", 
"     [PGUP]            moves a page up in top direction", 
"", 
"     [PGDOWN]          moves a page down in bottom direction", 
"", 
"     [g]               jumps to the top item", 
"", 
"     [G]               jumps to the bottom item", 
"", 
"     [i]               marks a port for installation", 
"", 
"     [u]               marks a port for upgrade", 
"", 
"     [d]               marks an installed port for deinstallation", 
"", 
"     [SPACE]           deselects a marked port or toggles install", 
"                       state, if it was not selected (like i)", 
"", 
"     [p]               proceeds action (install/upgrade/deinstall)", 
"", 
"     [q]               quits current browser, if categories were active,", 
"                       portsman is exit", 
"", 
"     [/]               search keyword in forward direction", 
"", 
"     [n]               search next occurence of keyword in forward", 
"                       direction", 
"", 
"     [?]               search keyword in backward direction", 
"", 
"     [N]               search next occurence of keyword in backward", 
"                       direction", 
"", 
"     [t]               toggles to terminal mode (return to portsman", 
"                       with ENTER)", 
"", 
"     [o]               open options browser if port is marked for", 
"                       install [i] or update [u]", 
"", 
"     [s]               summarizes all available internal information", 
"                       of a port", 
"", 
"     [f]               filter ports", 
"", 
"     [m]               mark all ports of current category with", 
"                       (i)nstall,  (u)pdate or (d)einstall flag,", 
"                       or deselects all marked ports if possible", 
"", 
"     [P]               toggles to category browsing with only physical", 
"                       categories (under your ports directory)", 
"                       (except: All and Installed categories)", 
"", 
"     [A]               toggles to category browsing with all physical", 
"                       and meta categories", 
"", 
"", 
" CATEGORY BROWSER", 
"", 
"     [ ]               category without any marked ports", 
"", 
"     [-]               category with marked ports for deinstallation", 
"", 
"     [+]               category with marked ports for installation", 
"                       or upgrade", 
"", 
"     [*]               category with marked ports for installation or", 
"                       upgrade and deinstallation", 
"", 
"     right column or status bar example:", 
"     <M>  -  1 / +  3 /   12 / 6765 ", 
"                       This means: <M> means meta category,"
"                                   <P> means physical category,"
"                                    1 port marked for deinstallation,", 
"                                    3 ports marked for installation or", 
"                                    update,", 
"                                    12 installed ports of 6765 available", 
"                                    ports", 
"", 
"", 
" PORTS BROWSER", 
"", 
"     [ ]               port is not installed", 
"", 
"     [=]               port is installed, the current version",  
"                       of the port is equal to the ports collection", 
"                       version", 
"", 
"     [<]               port is (maybe) installed, there's a newer", 
"                       version of the port in the ports collection,", 
"                       than the (maybe) installed version", 
"", 
"     [>]               ports is (maybe) installed, there's a older", 
"                       version of the port in the ports collection,", 
"                       than the (maybe) installed version", 
"", 
"     [R]               port is a runtime dependency for another",  
"                       as installation or upgrade marked port", 
"", 
"     [B]               port is a build dependency for another", 
"                       as installation or upgrade marked port", 
"",                        
"     [i]               port is marked for installation", 
"", 
"     [u]               port is marked for upgrade", 
"", 
"     [d]               port is marked for deinstallation", 
"",                       
"", 
" COMPILE OPTION BROWSER", 
"", 
"     < >               option not selected", 
"", 
"     <X>               option selected", 
"", 
"", 
" PROCEED (DE)INSTALL/UPDATE BROWSER", 
"", 
"     This browser is a mixture of PORTS BROWSER and COMPILE", 
"     OPTION BROWSER. It's behavior is equivalent to those both", 
"     browsers.", 
"", 
"", 
" SEE ALSO", 
"", 
"     portsman(1) and portsmanrc(5) manual pages.", 
"",  
};

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
#define _VERSION "portsman 0.2"
#define MAX_PATH 256
#define MAX_TOKEN 256
#define MAX_COLS 80
#define INSTALLED_PKG_DIR "/var/db/pkg"
#define INDEX_FILE "/usr/ports/INDEX"
#define PORTS_DIR "/usr/ports"
#define CONFIG_FILE ".portsmanrc"
#define REFRESH_WINDOW 0
#define REFRESH_ENTRY 1
#define STRING 1
#define STATE 0
#define STATE_NOT_SELECTED 0
#define STATE_NOT_INSTALLED 0 /* plist */
#define STATE_INSTALL 1
#define STATE_SELECTED 1
#define STATE_UPDATE 2
#define STATE_DEINSTALL 3
#define STATE_BDEP 4
#define STATE_RDEP 5
#define STATE_DIRECTORY 6 /* plist */
#define STATE_INSTALLED 7
#define STATE_INSTALLED_OLDER 8
#define STATE_INSTALLED_NEWER 9
#define ERROR_OPEN_INDEX -1
#define ERROR_CORRUPT_RC_FILE -1
#define ERROR_NO_RC_FILE -2
#define DEBUG
#define DEBUG_FILE "debug.log"
#define PORT 0
#define CATEGORY 1
#define OPTION 2
#define LINE 3
#define CLR_TITLE 0
#define CLR_BROWSE 1
#define CLR_SELECTOR 2
#define CLR_STATUS 3
#define CLR_CMD 4
#define MK_TARGET_INST 0
#define MK_TARGET_DEINST 1
#define MK_TARGET_UPDATE 2
#define MK_OPTION_FORCE 0
#define MK_OPTION_PKG 1
#define MK_OPTION_CLEAN 2
#define MK_OPTION_NOCHKSUM 3
#define MK_OPTION_NODEPS 4
#define MK_OPTION_FORCEPKGREG 5
#define MK_OPTION_NOPKGREG 6
#define HELP "help"
#define BROWSE_WITH_META_CATEGORIES 100
#define BROWSE_WITHOUT_META_CATEGORIES 200

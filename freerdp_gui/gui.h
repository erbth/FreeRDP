/**
 * Some gui functions for xfreerdp.
 *
 * Copyright 2014 Thomas Erbesdobler <t.erbesdobler@team103.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _GUI_H_
#define _GUI_H_

#define WLOGON_WIDTH 380
#define WLOGON_HEIGHT 200
#define WLOGON_MAXTAB 4

#define WERROR_WIDTH 400
#define WERROR_HEIGHT 80
#define WERROR_MAXTAB 1

#define WCERT_WIDTH 550
#define WCERT_HEIGHT 300
#define WCERT_MAXTAB 2

#define TEXTBOX_MAXLEN 400
#define PASSWORD_CHAR '*'


#include "X-tools.h"

/*prototypes*/
extern void logon_process_event(XEvent *report);
extern void logon_update_tab(XEvent *report);
extern void logon_blink_textboxes(void);
extern int logon_dialog(char *user,char *passwd);

extern void error_eventloop(void);
extern void error_process_event(XEvent *report);
extern void error_update_tab(XEvent *report);
extern int logon_error(void);

extern void cert_eventloop(int buttons);
extern void cert_process_event(XEvent *report,int buttons);
extern void cert_update_tab(XEvent *report,int buttons);
extern int cert_error(char *value,int buttons);

/*global variables*/
static Window wlogon,werror,wcert;
static struct button btabort,btok,btyes,btno;
static struct textbox txtuser,txtpasswd;
static struct eyes theeyes;

static char *cert_text;
#endif
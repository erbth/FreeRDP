/**
 * An smart toolkit for drawing basically windows, buttons, textboxes AND  some kind of xeyes.
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
 * 
 * NOTICE
 *	For computing the eyes pupils position the math segment from xeyes was taken:
 *		Copyright (c) 1991  X Consortium
 *
 *		Permission is hereby granted, free of charge, to any person obtaining
 *		a copy of this software and associated documentation files (the
 *		"Software"), to deal in the Software without restriction, including
 *		without limitation the rights to use, copy, modify, merge, publish,
 *		distribute, sublicense, and/or sell copies of the Software, and to
 *		permit persons to whom the Software is furnished to do so, subject to
 *		the following conditions:
 *
 *		The above copyright notice and this permission notice shall be included
 *		in all copies or substantial portions of the Software.
 *
 *		THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *		OR IMPLIED, INCLUDI-shareNG BUT NOT LIMITED TO THE WARRANTIES OF
 *		MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *		IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
 *		OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 *		ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *		Except as contained in this notice, the name of the X Consortium shall
 *		not be used in advertising or otherwise to promote the sale, use or
 *		other dealings in this Software without prior written authorization
 *		from the X Consortium.
 */

#ifndef X_TOOLS_H
#define X_TOOLS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "delay.h"
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <sys/time.h>
#include <math.h>

#define PIXELS_COUNT 7
#ifndef TEXTBOX_MAXLEN
	#define TEXTBOX_MAXLEN 200
	#warning "'TEXTBOX_MAXLEN' wasn't defined. redifining with 200"
#endif
#ifndef PASSWORD_CHAR
	#define PASSWORD_CHAR '*'
	#warning "'PASSWORD_CHAR' wasn't defined. redifining with '*'"
#endif

#define DIST_EYES 10
#define EYELINER_EYES 5
#define PUPILSIZE_EYES 6

/*global variables*/
Display *display;
int screen;
Screen *screeninfo;

Window rootwin;
GC mygc;
Font font,dialog_font,dialogdefault_font,text_font;
XFontStruct *fontinfo,*dialog_fontinfo,*dialogdefault_fontinfo,*text_fontinfo;
Colormap cmap;
unsigned long pixels[PIXELS_COUNT];enum{bg,bt_press,bt_release,text,bg_dialog,eyes,text_deselected};
char quit,blink_count;
Cursor dialog_cursor;
int tab;

Atom wmDeleteMessage;

struct button{
	int posx,posy;
	int width,height;
	char *text;
	int len;
	int textx,texty;
	char state; /*clickreturn,active,cursor,pressed*/
	char tabindex;
};

struct textbox{
	int posx,posy;
	int width,height;
	char *text,*defaulttext;
	int len,defaultlen;
	int textx,texty,textl;
	int scroll;
	int scroll_end;
	int cursor;
	char state;	/*passwd,blinkon,active,cursor*/
	char tabindex;
};

struct eyes{
	struct eye{
		int posx,posy;
		int width,height;
		int eyeliner;
		int pupilwidth,pupilheight;
		int pupilx;
		int pupily;
	}e[2];
};


/*prototypes*/
extern char xopen(void);
extern void xclose(void);

extern void init_button(struct button *bt,char *text,int posx,int posy,int width,int height,char clickreturn,char tab);
extern void draw_button(Window mywin,struct button *bt);
extern char update_button(Window mywin,struct button *bt,char release,int x,int y);
extern void cursor_button(Window mywin,struct button *bt,int x,int y);
extern void tab_button(Window mywin,struct button *bt);
extern char key_button(Window mywin,struct button *bt,XEvent *report,char release);
extern void free_button(struct button *bt);

extern void init_textbox(struct textbox *txt,int posx,int posy,int width,int height,char *defaulttext,char passwd,char tab);
extern void draw_textbox(Window mywin,struct textbox *txt);
extern void cursor_textbox(Window mywin,struct textbox *txt,int x,int y);
extern void blink_textbox(Window mywin,struct textbox *txt);
extern void click_textbox(Window mywin,struct textbox *txt,int x,int y);
extern void update_textbox(Window mywin,struct textbox *txt,XEvent *report);
extern void right_scroll_textbox(struct textbox *txt);
extern void left_scroll_textbox(struct textbox *txt);
extern void tab_textbox(Window mywin,struct textbox *txt);
extern void free_textbox(struct textbox *txt);

extern void draw_text(Window mywin,char *text,int x,int y);
extern void draw_itext(Window mywin,char *value,int x,int y,int width);

extern void create_window(Window *mywin,char *title,char *logo,int posx,int posy,int width,int height);
extern void close_window(Window mywin);

extern void init_eyes(struct eyes *myeyesi,int x,int y,int width,int height);
#define draw_eyes(mywin,myeyes) {draw_eye(mywin,myeyes,0);draw_eye(mywin,myeyes,1);}
extern void draw_eye(Window mywin,struct eyes *myeyes,int num);
#define update_eyes(mywin,myeyes,posx,posy) {update_eye(mywin,myeyes,0,posx,posy);update_eye(mywin,myeyes,1,posx,posy);draw_eyes(mywin,myeyes);}
extern void update_eye(Window mywin,struct eyes *myeyes,int num,int posx,int posy);
extern void textbox_eyes(Window mywin,struct eyes *myeyes,struct textbox *mytxt);
extern void button_eyes(Window mywin,struct eyes *myeyes,struct button *bt);

#endif
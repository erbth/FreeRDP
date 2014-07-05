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
 *		OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *		MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *		IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
 *		OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 *		ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *		Except as contained in this notice, the name of the X Consortium shall
 *		not be used in advertising or otherwise to promote the sale, use or
 *		other dealings in this Software without prior written authorization
 *		from the X Consortium.
 */

/* basic X toolkit for drawing windows, buttons and textboxes

usage:	-xopen();
	-create_window();
	-e.g. init-button() or init_textbox()
	-process mainloop
	-e.g. free_button() or free_textbox()
	-close_window();
	-xclose();

restrictions:
	-only ONE window can exist at once!
*/

#include "X-tools.h"

/*functions*/
char UTF8toASCII(char *data){
	if(*data=='\303'){
		switch(*(data+1)){
			case '\266':
				return (char)246;
				break;
			case '\244':
				return (char)228;
				break;
			case '\274':
				return (char)252;
				break;
				
			case '\226':
				return (char)214;
				break;
			case '\204':
				return (char)196;
				break;
			case '\234':
				return (char)220;
				break;
				
			case '\237':
				return (char)223;
				break;
				
			default:
				break;
		}
	}
	if((*data=='\302')&&(*(data+1)=='\260')){
		return (char)176;
	}
	
	return '?';
}


char xopen(void){
	XColor mycolor;
	Status result;

	display=XOpenDisplay(NULL);
	if(display==NULL){
		return 1;
	}

	screen=XDefaultScreen(display);
	screeninfo=XDefaultScreenOfDisplay(display);
	
	if(XDefaultDepth(display,screen)<2){
		XCloseDisplay(display);
		return 2;
	}

	XFlush(display);

	cmap=DefaultColormap(display,screen);
	mycolor.red=49858;mycolor.green=48059;mycolor.blue=47288;
	result=XAllocColor(display,cmap,&mycolor);
	if(result==0){
		XCloseDisplay(display);
		return 2;
	}
	pixels[bg]=mycolor.pixel;

	cmap=DefaultColormap(display,screen);
	mycolor.red=43433;mycolor.green=43433;mycolor.blue=43433;
	result=XAllocColor(display,cmap,&mycolor);
	if(result==0){
		XFreeColors(display,cmap,pixels,1,0);
		XCloseDisplay(display);
		return 2;
	}
	pixels[bt_press]=mycolor.pixel;

	cmap=DefaultColormap(display,screen);
	mycolor.red=59367;mycolor.green=59367;mycolor.blue=59367;
	result=XAllocColor(display,cmap,&mycolor);
	if(result==0){
		XFreeColors(display,cmap,pixels,2,0);
		XCloseDisplay(display);
		return 2;
	}
	pixels[bt_release]=mycolor.pixel;

	cmap=DefaultColormap(display,screen);
	mycolor.red=0;mycolor.green=0;mycolor.blue=0;
	result=XAllocColor(display,cmap,&mycolor);
	if(result==0){
		XFreeColors(display,cmap,pixels,3,0);
		XCloseDisplay(display);
		return 2;
	}
	pixels[text]=mycolor.pixel;

	cmap=DefaultColormap(display,screen);
	mycolor.red=65535;mycolor.green=65535;mycolor.blue=65535;
	result=XAllocColor(display,cmap,&mycolor);
	if(result==0){
		XFreeColors(display,cmap,pixels,4,0);
		XCloseDisplay(display);
		return 2;
	}
	pixels[bg_dialog]=mycolor.pixel;

	cmap=DefaultColormap(display,screen);
	mycolor.red=0;mycolor.green=0;mycolor.blue=0;
	result=XAllocColor(display,cmap,&mycolor);
	if(result==0){
		XFreeColors(display,cmap,pixels,5,0);
		XCloseDisplay(display);
		return 2;
	}
	pixels[eyes]=mycolor.pixel;

	cmap=DefaultColormap(display,screen);
	mycolor.red=26984;mycolor.green=25699;mycolor.blue=26727;
	result=XAllocColor(display,cmap,&mycolor);
	if(result==0){
		XFreeColors(display,cmap,pixels,6,0);
		XCloseDisplay(display);
		return 2;
	}
	pixels[text_deselected]=mycolor.pixel;

	font=XLoadFont(display,"*helvetica-medium-r-normal--11-*-iso8859-1");
	fontinfo=XQueryFont(display,font);


	dialog_font=XLoadFont(display,"*helvetica-medium-r-normal--12-*-iso8859-1");
	dialog_fontinfo=XQueryFont(display,dialog_font);
	
	dialogdefault_font=XLoadFont(display,"*helvetica-medium-o-normal--12-*-iso8859-1");
	dialogdefault_fontinfo=XQueryFont(display,dialog_font);

	
	text_font=XLoadFont(display,"*helvetica-medium-r-normal--12-*-iso8859-1");
	text_fontinfo=XQueryFont(display,dialog_font);


	dialog_cursor=XCreateFontCursor(display,XC_xterm);

	rootwin=RootWindow(display,screen);
	
	tab=0;
	
	wmDeleteMessage=XInternAtom(display,"WM_DELETE_WINDOW",0);

	return 0;
}

void xclose(void){
	XUnloadFont(display,font);
	XUnloadFont(display,dialog_font);
	XUnloadFont(display,text_font);
	XFreeColors(display,cmap,pixels,PIXELS_COUNT,0);
	XFreeCursor(display,dialog_cursor);
	XCloseDisplay(display);
	return;
}


void init_button(struct button *bt,char *text,int posx,int posy,int width,int height,char clickreturn,char tab){
	int len,temp1;

	len=strlen(text);

/*check dimensions*/
	temp1=(fontinfo->ascent+fontinfo->descent);
	if(temp1>(height-6)){
		height=temp1+6;
	}

	temp1=(XTextWidth(fontinfo,text,len));
	if(temp1>(width-6)){
		width=temp1+6;
	}

	bt->posx=posx;
	bt->posy=posy;
	bt->width=width;
	bt->height=height;

	bt->text=malloc(sizeof(char)*(len+1));
	strcpy(bt->text,text);
	bt->len=len;
	bt->textx=posx+((width/2)-(XTextWidth(fontinfo,text,len)/2));
	bt->texty=posy+((height/2)+((fontinfo->ascent-fontinfo->descent)/2));

	bt->state=0;
	if(clickreturn){
		bt->state|=0x08;
	}
	
	bt->tabindex=tab;
}

void draw_button(Window mywin,struct button *bt){
	if(!(bt->state&0x01)){
		XSetForeground(display,mygc,pixels[bt_release]);
	}else{
		XSetForeground(display,mygc,pixels[bt_press]);
	}

	XFlush(display);

	XFillRectangle(display,mywin,mygc,(bt->posx+3),bt->posy,(bt->width-6),bt->height);
	XFillRectangle(display,mywin,mygc,bt->posx,(bt->posy+3),3,(bt->height-6));
	XFillRectangle(display,mywin,mygc,(bt->posx+bt->width-3),(bt->posy+3),3,(bt->height-6));

	XFillArc(display,mywin,mygc,(bt->posx),(bt->posy),6,6,90*64,90*64);
	XFillArc(display,mywin,mygc,(bt->posx),(bt->posy+bt->height-6),6,6,180*64,90*64);
	XFillArc(display,mywin,mygc,(bt->posx+bt->width-6),(bt->posy+bt->height-6),6,6,270*64,90*64);
	XFillArc(display,mywin,mygc,(bt->posx+bt->width-6),(bt->posy),6,6,0*64,90*64);

	if((bt->state&0x02)||(bt->state&0x04)){
		XSetForeground(display,mygc,BlackPixel(display,screen));
	}else{
		XSetForeground(display,mygc,WhitePixel(display,screen));
	}
	XDrawArc(display,mywin,mygc,(bt->posx),(bt->posy),6,6,90*64,90*64);
	XDrawArc(display,mywin,mygc,(bt->posx),(bt->posy+bt->height-6),6,6,180*64,90*64);
	XDrawArc(display,mywin,mygc,(bt->posx+bt->width-6),(bt->posy+bt->height-6),6,6,270*64,90*64);
	XDrawArc(display,mywin,mygc,(bt->posx+bt->width-6),(bt->posy),6,6,0*64,90*64);

	XDrawLine(display,mywin,mygc,(bt->posx),(bt->posy+3),(bt->posx),(bt->posy+bt->height-3));
	XDrawLine(display,mywin,mygc,(bt->posx+3),(bt->posy+bt->height),(bt->posx+bt->width-3),(bt->posy+bt->height));
	XDrawLine(display,mywin,mygc,(bt->posx+bt->width),(bt->posy+bt->height-3),(bt->posx+bt->width),(bt->posy+3));
	XDrawLine(display,mywin,mygc,(bt->posx+bt->width-3),(bt->posy),(bt->posx+3),(bt->posy));


	XSetForeground(display,mygc,pixels[text]);
	XSetFont(display,mygc,font);
	XDrawString(display,mywin,mygc,bt->textx,bt->texty,bt->text,bt->len);
	return;
}

char update_button(Window mywin,struct button *bt,char release,int x,int y){
	if((x>(bt->posx+2))&&(x<(bt->posx+bt->width-2))&&(y>(bt->posy+2))&&(y<(bt->posy+bt->height-2))){
		tab=bt->tabindex-1;

		if(release){
			bt->state&=~0x01;
			draw_button(mywin,bt);
			return 1;
		}else{
			bt->state|=0x04;
			bt->state|=0x01;
			draw_button(mywin,bt);
			return 0;
		}
	}else{
		if(!release){
			bt->state&=~0x04;
		}

		bt->state&=~0x01;
		draw_button(mywin,bt);
		return 0;
	}
}

void cursor_button(Window mywin,struct button *bt,int x,int y){
	if((x>(bt->posx+2))&&(x<(bt->posx+bt->width-2))&&(y>(bt->posy+2))&&(y<(bt->posy+bt->height-2))){
		bt->state|=0x02;
	}else{
		bt->state&=~0x02;
	}
	draw_button(mywin,bt);
	return;
}

void tab_button(Window mywin,struct button *bt){
	if(bt->tabindex==tab){
		bt->state|=0x04;
	}else{
		bt->state&=~0x04;
	}
	draw_button(mywin,bt);
	return;
}

char key_button(Window mywin,struct button *bt,XEvent *report,char release){
	KeySym key;
	char letter;
	
	XLookupString(&(report->xkey),&letter,1,&key,NULL);
	if(release){
		if((((bt->state&0x08)&&(tab<=bt->tabindex))||(bt->state&0x04))&&(letter==0x0D)){
			if(bt->state&0x01){
				bt->state&=~0x01;
				draw_button(mywin,bt);
				return 1;
			}
		}
	}else{
		if((((bt->state&0x08)&&(tab<=bt->tabindex))||(bt->state&0x04))&&(letter==0x0D)){
			bt->state|=0x01;
		}
	}
	draw_button(mywin,bt);
	return 0;
}

void free_button(struct button *bt){
	free(bt->text);
	return;
}


void init_textbox(struct textbox *txt,int posx,int posy,int width,int height,char *defaulttext,char passwd,char tab){
	int temp1;
	int defaultlen;
	
	defaultlen=strlen(defaulttext)+1;

	temp1=(dialog_fontinfo->ascent+dialog_fontinfo->descent);
	if(temp1>(height-6)){
		height=temp1+6;
	}


	txt->posx=posx;
	txt->posy=posy;
	txt->width=width;
	txt->height=height;
	txt->len=0;
	txt->scroll=0;
	txt->scroll_end=0;
	txt->cursor=0;
	txt->text=malloc(sizeof(char)*(TEXTBOX_MAXLEN+1));
	*(txt->text)=0;
	
	txt->defaulttext=malloc(sizeof(char)*(defaultlen));
	strncpy(txt->defaulttext,defaulttext,defaultlen);
	
	txt->defaultlen=defaultlen-1;

	txt->textx=posx+3;
	txt->textl=posx+width-3;
	txt->texty=posy+((height/2)+((dialog_fontinfo->ascent-dialog_fontinfo->descent)/2));

	txt->state=0;
	if(passwd){
		txt->state|=0x08;
	}
	
	txt->tabindex=tab;

	return;
}

void draw_textbox(Window mywin,struct textbox *txt){
	int temp1,i;
	char *tempcp1;
	
	XSetForeground(display,mygc,pixels[bg_dialog]);

	XFillRectangle(display,mywin,mygc,(txt->posx+3),(txt->posy),(txt->width-6),(txt->height));
	XFillRectangle(display,mywin,mygc,(txt->posx),(txt->posy+3),3,(txt->height-6));
	XFillRectangle(display,mywin,mygc,(txt->posx+txt->width-3),(txt->posy+3),3,(txt->height-3));

	XFillArc(display,mywin,mygc,(txt->posx),(txt->posy),6,6,90*64,90*64);
	XFillArc(display,mywin,mygc,(txt->posx),(txt->posy+txt->height-6),6,6,180*64,90*64);
	XFillArc(display,mywin,mygc,(txt->posx+txt->width-6),(txt->posy+txt->height-6),6,6,270*64,90*64);
	XFillArc(display,mywin,mygc,(txt->posx+txt->width-6),(txt->posy),6,6,0*64,90*64);


	XSetForeground(display,mygc,BlackPixel(display,screen));

	XDrawLine(display,mywin,mygc,(txt->posx),(txt->posy+3),(txt->posx),(txt->posy+txt->height-3));
	XDrawLine(display,mywin,mygc,(txt->posx+3),(txt->posy+txt->height),(txt->posx+txt->width-3),(txt->posy+txt->height));
	XDrawLine(display,mywin,mygc,(txt->posx+txt->width),(txt->posy+txt->height-3),(txt->posx+txt->width),(txt->posy+3));
	XDrawLine(display,mywin,mygc,(txt->posx+txt->width-3),(txt->posy),(txt->posx+3),(txt->posy));

	XDrawArc(display,mywin,mygc,(txt->posx),(txt->posy),6,6,90*64,90*64);
	XDrawArc(display,mywin,mygc,(txt->posx),(txt->posy+txt->height-6),6,6,180*64,90*64);
	XDrawArc(display,mywin,mygc,(txt->posx+txt->width-6),(txt->posy+txt->height-6),6,6,270*64,90*64);
	XDrawArc(display,mywin,mygc,(txt->posx+txt->width-6),(txt->posy),6,6,0*64,90*64);

	if(txt->len>0){
		XSetFont(display,mygc,dialog_font);
		XSetForeground(display,mygc,pixels[text]);

		if(txt->state&0x08){
			temp1=(txt->scroll_end-txt->scroll);
			tempcp1=malloc(sizeof(char)*temp1);

			for(i=0;i<temp1;i++){
				*(tempcp1+i)=PASSWORD_CHAR;
			}

			XDrawString(display,mywin,mygc,txt->textx,txt->texty,tempcp1,temp1);
			
			XSetForeground(display,mygc,BlackPixel(display,screen));
			if(txt->state&0x04){
				temp1=(txt->posx+3+XTextWidth(dialog_fontinfo,tempcp1,(txt->cursor-txt->scroll)));
				XDrawLine(display,mywin,mygc,temp1,(txt->posy+2),temp1,(txt->posy+txt->height-2));
			}

			free(tempcp1);
		}else{
			XDrawString(display,mywin,mygc,txt->textx,txt->texty,(txt->text+txt->scroll),(txt->scroll_end-txt->scroll));

			XSetForeground(display,mygc,BlackPixel(display,screen));
			if(txt->state&0x04){
				temp1=(txt->posx+3+XTextWidth(dialog_fontinfo,(txt->text+txt->scroll),(txt->cursor-txt->scroll)));
				XDrawLine(display,mywin,mygc,temp1,(txt->posy+2),temp1,(txt->posy+txt->height-2));
			}
		}
	}else{
		XSetFont(display,mygc,dialogdefault_font);
		XSetForeground(display,mygc,pixels[text_deselected]);
		XDrawString(display,mywin,mygc,txt->textx,txt->texty,txt->defaulttext,txt->defaultlen);
		
		XSetForeground(display,mygc,BlackPixel(display,screen));
		if(txt->state&0x04){
			temp1=(txt->posx+3);
			XDrawLine(display,mywin,mygc,temp1,(txt->posy+2),temp1,(txt->posy+txt->height-2));
		}
	}



	return;
}

void cursor_textbox(Window mywin,struct textbox *txt,int x,int y){
	if((x>(txt->posx+2))&&(x<(txt->posx+txt->width-2))&&(y>(txt->posy+2))&&(y<(txt->posy+txt->height-1))){
		if((txt->state&0x01)==0){
			XDefineCursor(display,mywin,dialog_cursor);
			txt->state|=0x01;
		}
	}else{
		if((txt->state&0x01)!=0){
			XUndefineCursor(display,mywin);
			XFlush(display);
			txt->state&=~0x01;
		}
	}
	return;
}

void blink_textbox(Window mywin,struct textbox *txt){
	if(txt->state&0x02){
		if(txt->state&0x04){
			txt->state&=~0x04;
		}else{
			txt->state|=0x04;
		}
		draw_textbox(mywin,txt);
	}
	return;
}

void click_textbox(Window mywin,struct textbox *txt,int x,int y){
	int i,temp1,temp2;
	char buf;
	
	if((x>(txt->posx+2))&&(x<(txt->posx+txt->width-2))&&(y>(txt->posy+2))&&(y<(txt->posy+txt->height-1))){
		buf=PASSWORD_CHAR;
		txt->state|=0x02;
		txt->state|=0x04;

		temp1=0;
		temp2=x-txt->posx;
		for(i=txt->scroll;temp2>temp1;i++){
			if(txt->state&0x08){
				temp1=temp1+XTextWidth(dialog_fontinfo,&buf,1);
			}else{
				temp1=XTextWidth(dialog_fontinfo,(txt->text+txt->scroll),(i-txt->scroll));
			}
		}
		i-=2;

		if(i>txt->scroll_end){
			i=txt->scroll_end;
		}

		txt->cursor=i;
		
		tab=txt->tabindex-1;
	}else{
		txt->state&=~0x02;
		txt->state&=~0x04;
	}
	draw_textbox(mywin,txt);
	return;
}

void update_textbox(Window mywin,struct textbox *txt,XEvent *report){
	char letter[2];
	int i,len;
	char buf;
	KeySym key;

	if(txt->state&0x02){
		txt->state|=0x04;
		blink_count=0;

		XLookupString(&report->xkey,letter,2,&key,NULL);
		if(letter[0]!=0){
			if(((letter[0]>=0x20)/*&&(letter[0]<0x100)*/&&(letter[0]!=0x7F))||(letter[0]<0)){
				if(txt->len<TEXTBOX_MAXLEN){
					buf=PASSWORD_CHAR;

					if(letter[0]<0){
						letter[0]=UTF8toASCII(letter);
					}
					
					for(i=txt->len;i>txt->cursor;i--){
						*(txt->text+i)=*(txt->text+i-1);
					}

					*(txt->text+i)=letter[0];
					*(txt->text+txt->len+1)=0;

					txt->len++;
					txt->cursor++;

					if(txt->state&0x08){
						len=(txt->cursor-txt->scroll)*XTextWidth(dialog_fontinfo,&buf,1);
					}else{
						len=XTextWidth(dialog_fontinfo,(txt->text+txt->scroll),(txt->cursor-txt->scroll));
					}

					/*if*/while(len>(txt->width-8)){
						txt->scroll++;

						if(txt->state&0x08){
							len=(txt->cursor-txt->scroll)*XTextWidth(dialog_fontinfo,&buf,1);
						}else{
							len=XTextWidth(dialog_fontinfo,(txt->text+txt->scroll),(txt->cursor-txt->scroll));
						}
					}
					left_scroll_textbox(txt);
				}
			}

			if((letter[0]==0x08)&&(txt->cursor>0)){
				for(i=(txt->cursor-1);i<(txt->len-1);i++){
					*(txt->text+i)=*(txt->text+i+1);
				}
				*(txt->text+i)=0;

				txt->len--;
				txt->cursor--;
				txt->scroll_end--;

				if(txt->cursor<txt->scroll){
					txt->scroll--;
				}
				left_scroll_textbox(txt);
			}
			if((letter[0]==0x7F)&&(txt->cursor<txt->len)){
				for(i=(txt->cursor);i<(txt->len-1);i++){
					*(txt->text+i)=*(txt->text+i+1);
				}
				*(txt->text+i)=0;

				txt->len--;
				if(txt->scroll_end>txt->len){
					txt->scroll_end--;
				}
			}
		}


		if(XKeysymToKeycode(display,key)==113){
			if(txt->cursor>0){
				txt->cursor--;
				if(txt->cursor<txt->scroll){
					txt->scroll--;
					left_scroll_textbox(txt);
				}
			}
		}

		if(XKeysymToKeycode(display,key)==114){
			if(txt->cursor<txt->len){
				txt->cursor++;
				if(txt->cursor>txt->scroll_end){
					txt->scroll_end++;
					right_scroll_textbox(txt);
				}
			}
		}

		draw_textbox(mywin,txt);
	}	
	return;
}

void right_scroll_textbox(struct textbox *txt){
	int temp1,temp2=0;
	char buf;
	
	if(txt->state&0x08){
		buf=PASSWORD_CHAR;
		temp2=XTextWidth(dialog_fontinfo,&buf,1);
		
		temp1=-temp2;
	}else{
		temp1=0;
	}
	for(txt->scroll=txt->scroll_end;(temp1<(txt->width-8))&&(txt->scroll>=(0-1));txt->scroll--){ /*there's the problem of thetwo cases having either two or 1 LSB difference*/
		if(txt->state&0x08){
			temp1=temp1+temp2;
		}else{
			temp1=XTextWidth(dialog_fontinfo,(txt->text+txt->scroll),(txt->scroll_end-txt->scroll));/*temp1=width('*')+temp1*/
		}
	}
	txt->scroll+=2; /*this way and not by foot-controlled loop to get sure that txt->scroll doesn't get negative (in case of txt->len==0)*/
	return;
}

void left_scroll_textbox(struct textbox *txt){
	int temp1,temp2=0;
	char buf;
	
	if(txt->state&0x08){
		buf=PASSWORD_CHAR;
		temp2=XTextWidth(dialog_fontinfo,&buf,1);
		
		temp1=-temp2;
	}else{
		temp1=0;
	}
	for(txt->scroll_end=txt->scroll;(temp1<(txt->width-8))&&(txt->scroll_end<=(txt->len+1));txt->scroll_end++){
		if(txt->state&0x08){
			temp1=temp1+temp2;
		}else{
			temp1=XTextWidth(dialog_fontinfo,(txt->text+txt->scroll),(txt->scroll_end-txt->scroll));
		}
	}
	txt->scroll_end-=2; /*this way and not by foot-controlled loop to get sure that txt->scroll_end doesn't address not written to cells of vector txt->text (in case of txt->len==0)*/
	return;
}

void tab_textbox(Window mywin,struct textbox *txt){
	if(txt->tabindex==tab){
		txt->state|=(0x02|0x04);
	}else{
		txt->state&=~(0x02|0x04);
	}
	draw_textbox(mywin,txt);
	return;
}

void free_textbox(struct textbox *txt){
	free(txt->text);
	return;
}


void draw_text(Window mywin,char *value,int x,int y){
	XSetForeground(display,mygc,pixels[text]);
	XSetFont(display,mygc,text_font);
	
	XDrawString(display,mywin,mygc,x,y,value,strlen(value));
	return;
}

void draw_itext(Window mywin,char *value,int x,int y,int width){
	int j;
	int temp1,temp2,temp3,temp4;
	char *tempp,*realn;
	
	XSetForeground(display,mygc,pixels[text]);
	XSetFont(display,mygc,text_font);
	
	temp1=0;
	temp2=0;
	temp3=0;
	j=0;
	
	while(1){
		tempp=strchr(value+temp1,' ');
		realn=strchr(value+temp1,'\n');
		if((!tempp)&&(!realn))
			break;
		
		if(!tempp){
			tempp=realn;
		}else{
			if(realn){
				if(realn<tempp){
					tempp=realn;
				}else{
					realn=NULL;
				}
			}
		}

		temp2=tempp-value-temp3;
		if(realn){
			temp1=temp2+temp3+1;
		}
		
		if(realn||XTextWidth(text_fontinfo,value+temp3,temp2)>width){
			XDrawString(display,mywin,mygc,x,y+j*(text_fontinfo->ascent+text_fontinfo->descent),value+temp3,temp1-temp3-1);
			j++;
			
			temp4=temp1;
			temp2++;
			temp1=temp2+temp3;
			
			temp3=temp4;
		}else{
			temp2++;
			temp1=temp2+temp3;
		}
	}
	
	temp1=strlen(value);
	if(temp3<temp1){
		XDrawString(display,mywin,mygc,x,y+j*(text_fontinfo->ascent+text_fontinfo->descent),value+temp3,temp1-temp3);
	}
	
	return;
}


void create_window(Window *mywin,char *title,char *logo,int posx,int posy,int width,int height){
	XSizeHints size_hints;
	
	*mywin=XCreateSimpleWindow(display,rootwin,posx,posy,width,height,5,BlackPixel(display,screen),pixels[bg]);
	size_hints.flags=PSize|PMinSize|PMaxSize|PPosition;
	size_hints.min_width=width;
	size_hints.max_width=width;
	size_hints.min_height=height;
	size_hints.max_height=height;
	XSetStandardProperties(display,*mywin,title,logo,None,0,0,&size_hints);
	XSelectInput(display,*mywin,ButtonPressMask|ButtonReleaseMask|KeyPressMask|KeyReleaseMask|ExposureMask|PointerMotionMask);

	XMapWindow(display,*mywin);

	mygc=XCreateGC(display,*mywin,0,0);
	XSetFont(display,mygc,font);
	
	XSetWMProtocols(display,*mywin,&wmDeleteMessage,1);

	return;
}

void close_window(Window mywin){
	XFreeGC(display,mygc);
	XDestroyWindow(display,mywin);
	return;
}


void init_eyes(struct eyes *myeyes,int x,int y,int width,int height){
	myeyes->e[0].width=(width-width/DIST_EYES)/2;
	myeyes->e[1].width=(width-width/DIST_EYES)/2;
	
	myeyes->e[0].height=(height);
	myeyes->e[1].height=(height);
	
	myeyes->e[0].posx=x+myeyes->e[0].width/2;
	myeyes->e[1].posx=x+width-myeyes->e[1].width/2;
	
	myeyes->e[0].posy=y+height/2;
	myeyes->e[1].posy=y+height/2;
	
	myeyes->e[0].eyeliner=myeyes->e[0].width/EYELINER_EYES;
	myeyes->e[1].eyeliner=myeyes->e[1].width/EYELINER_EYES;
	
	myeyes->e[0].pupilwidth=myeyes->e[0].width/PUPILSIZE_EYES;
	myeyes->e[1].pupilwidth=myeyes->e[1].width/PUPILSIZE_EYES;
	
	myeyes->e[0].pupilheight=myeyes->e[0].height/PUPILSIZE_EYES;
	myeyes->e[1].pupilheight=myeyes->e[1].height/PUPILSIZE_EYES;
	
	myeyes->e[0].pupilx=myeyes->e[0].posx;
	myeyes->e[1].pupilx=myeyes->e[1].posx;
	
	myeyes->e[0].pupily=myeyes->e[0].posy;
	myeyes->e[1].pupily=myeyes->e[1].posy;
	
	return;
}

void draw_eye(Window mywin,struct eyes *myeyes,int num){
	double test1,test2=10.0;
	test1=sin( test2 );
	
	XSetForeground(display,mygc,pixels[eyes]);
	XFillArc(display,mywin,mygc,(myeyes->e[num].posx-myeyes->e[num].width/2),(myeyes->e[num].posy-myeyes->e[num].height/2),myeyes->e[num].width,myeyes->e[num].height,0,360*64);
	
	XSetForeground(display,mygc,WhitePixel(display,screen));
	XFillArc(display,mywin,mygc,(myeyes->e[num].posx-myeyes->e[num].width/2+myeyes->e[num].eyeliner*myeyes->e[num].width/myeyes->e[num].height),(myeyes->e[num].posy-myeyes->e[num].height/2+myeyes->e[num].eyeliner),myeyes->e[num].width-myeyes->e[num].eyeliner*myeyes->e[num].width/myeyes->e[num].height*2,myeyes->e[num].height-myeyes->e[num].eyeliner*2,0,360*64);
	
	XSetForeground(display,mygc,pixels[eyes]);
	XFillArc(display,mywin,mygc,myeyes->e[num].pupilx-myeyes->e[num].pupilwidth/2,myeyes->e[num].pupily-myeyes->e[num].pupilheight/2,myeyes->e[num].pupilwidth,myeyes->e[num].pupilheight,0,360*64);
	
	return;
}

void update_eye(Window mywin,struct eyes *myeyes,int num,int posx,int posy){
	int dist;
	double angle;
	double h;
	int x,y;
	int dx,dy;
	double cosa,sina;
	
	dx=posx-myeyes->e[num].posx;
	dy=posy-myeyes->e[num].posy;
	if(dx==0 && dy==0){
		myeyes->e[num].pupilx=myeyes->e[num].posx;
		myeyes->e[num].pupily=myeyes->e[num].posy;
	}else{
		angle=atan2((double)dy,(double)dx);
		cosa=cos(angle);
		sina=sin(angle);
		
		h=hypot((myeyes->e[num].height/2)*cosa,(myeyes->e[num].width/2)*sina);
		x=((myeyes->e[num].width/2)*(myeyes->e[num].height/2))*cosa/h;
		y=((myeyes->e[num].width/2)*(myeyes->e[num].height/2))*sina/h;

		dist=0.5*hypot(x,y);
		
		if(dist>hypot((double)dx,(double)dy)){
			myeyes->e[num].pupilx=dx+myeyes->e[num].posx;
			myeyes->e[num].pupily=dy+myeyes->e[num].posy;
		}else{
			myeyes->e[num].pupilx=(int)(dist*cosa)+myeyes->e[num].posx;
			myeyes->e[num].pupily=(int)(dist*sina)+myeyes->e[num].posy;
		}
	}
	return;
}

void textbox_eyes(Window mywin,struct eyes *myeyes,struct textbox *mytxt){
	int x,y;
	
	if(mytxt->state&0x02){
		x=mytxt->posx+XTextWidth(dialog_fontinfo,(mytxt->text+mytxt->scroll),(mytxt->cursor-mytxt->scroll));
		y=mytxt->posy+(mytxt->height/2);
	
		update_eyes(mywin,myeyes,x,y);
	}
	return;
}

void button_eyes(Window mywin,struct eyes *myeyes,struct button *bt){
	if(bt->state&0x04){
		update_eyes(mywin,myeyes,(bt->posx+bt->width/2),(bt->posy+bt->height/2));
	}
}

#if 0
/* an examplary blink_textboxes() function
void blink_textboxes(void){
	blink_textbox(wlogon,&txtuser);
	blink_textbox(wlogon,&txtpasswd);
	return;
}
*/
/* an examplary eventloop
void eventloop(void){
	XEvent xev;
	int num_events;

	XFlush(display);
	num_events=XPending(display);
	while(num_events!=0){
		num_events--;
		
		XNextEvent(display,&xev);
		process_event(&xev);
	}
	return;
}

void process_event(XEvent *report){
	char result;

	if(report->type==Expose){
		draw_button(wlogon,&btabort);
		draw_button(wlogon,&btok);
		draw_text(wlogon,"Bitte geben Sie Benutzername und Passwort ein:,10,15);
		draw_textbox(wlogon,&txtuser);
		draw_textbox(wlogon,&txtpasswd);
	}

	if(report->type==ButtonPress){
		if(report->xbutton.button==1){
			update_button(wlogon,&btabort,0,report->xbutton.x,report->xbutton.y);
			update_button(wlogon,&btok,0,report->xbutton.x,report->xbutton.y);
		}
		click_textbox(wlogon,&txtuser,report->xbutton.x,report->xbutton.y);
		click_textbox(wlogon,&txtpasswd,report->xbutton.x,report->xbutton.y);
	}

	if(report->type==ButtonRelease){
		if(report->xbutton.button==1){
			result=update_button(wlogon,&btabort,1,report->xbutton.x,report->xbutton.y);
			if(result){
				quit=1;
			}

			result=update_button(wlogon,&btok,1,report->xbutton.x,report->xbutton.y);
			if(result){
				printf("user:     %s\npassword: %s\n",txtuser.text,txtpasswd.text);
			}
		}
	}

	if(report->type==MotionNotify){
		cursor_textbox(wlogon,&txtuser,report->xmotion.x,report->xmotion.y);
		cursor_textbox(wlogon,&txtpasswd,report->xmotion.x,report->xmotion.y);
	}

	if(report->type==KeyPress){
		update_textbox(wlogon,&txtuser,report);
		update_textbox(wlogon,&txtpasswd,report);
	}
}

*/

/* an examplary mainloop
int main(int argc,char *argv){
	int td,pause=0;

	if(xopen()){
		printf("Connection to XServer couldn't be established.");
		return EXIT_FAILURE;
	}
	create_window(wlogon,"Anmeldedaten","window",100,100,400,300);
	init_button(&btabort,"Abbrechen",200,200,60,0);
	init_button(&btok,"OK",100,200,60,0);
	init_textbox(&txtuser,20,60,150,0,0);
	init_textbox(&txtpasswd,20,100,150,0,1);

	quit=0;
	blink_count=0;
	gettimeofday(&st,NULL);
	while(!quit){
		td=time_diff();
		pause=delay(FRAME_LEN-td+pause); //wait 10ms for saving cpu ticks
		
		if(blink_count>=50){
			blink_count=0;
			blink_textboxes();
		}
		blink_count++;

		eventloop();
	}

	close_window(wlogon);
	free_button(&btabort);
	free_button(&btok);
	free_textbox(&txtuser);
	free_textbox(&txtpasswd);

	xclose();

	return EXIT_SUCCESS;
}
*/
#endif

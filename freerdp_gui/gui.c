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

#ifndef _GUI_C_
#define _GUI_C_

#include "gui.h"

/*functions*/
void ASCIItoUTF8(char *data){
	int i,j,temp1;
	char buf;
	
	temp1=strlen(data);
	for(i=0;i<temp1;i++){
		buf=data[i];
		if((buf<0)&&(buf!='\303')&&(buf!='\302')){
			for(j=temp1;j>(i+1);j--){
				data[j]=data[j-1];
			}
			temp1++;
			data[temp1]=0;
			
			switch(buf){
				case (char)246:
					data[i]='\303';
					data[i+1]='\266';
					break;
				case (char)228:
					data[i]='\303';
					data[i+1]='\244';
					break;
				case (char)252:
					data[i]='\303';
					data[i+1]='\274';
					break;
					
				case (char)214:
					data[i]='\303';
					data[i+1]='\226';
					break;
				case (char)196:
					data[i]='\303';
					data[i+1]='\204';
					break;
				case (char)220:
					data[i]='\303';
					data[i+1]='\234';
					break;
					
				case (char)223:
					data[i]='\303';
					data[i+1]='\237';
					break;
				case (char)176:
					data[i]='\302';
					data[i+1]='\260';
					break;
				
					
				default:
					data[i]='?';
					data[i+1]='?';
					break;
			}
			i++;
		}
	}
	
	return;
}



void logon_blink_textboxes(void){
	blink_textbox(wlogon,&txtuser);
	blink_textbox(wlogon,&txtpasswd);
	return;
}


#define logon_pressok quit=1

void logon_eventloop(void){
	XEvent xev;
	int num_events;
	Window root,child;
	int rootx,rooty,x,y;
	static int xold,yold;
	unsigned int mask;

	XFlush(display);
	num_events=XPending(display);
	while(num_events!=0){
		num_events--;
		
		XNextEvent(display,&xev);
		logon_process_event(&xev);
	}
	
	XQueryPointer(display,wlogon,&root,&child,&rootx,&rooty,&x,&y,&mask);
	if(xold!=x&&yold!=y)
		update_eyes(wlogon,&theeyes,x,y);
	xold=x;
	yold=y;
	return;
}

void logon_process_event(XEvent *report){
	char result;

	if(report->type==ClientMessage){
		if(report->xclient.data.l[0]==wmDeleteMessage){
			quit=2;
		}
	}


	if(report->type==Expose){
		draw_button(wlogon,&btabort);
		draw_button(wlogon,&btok);
		draw_text(wlogon,"Bitte geben Sie Benutzername und Passwort ein:",10,15);
		draw_textbox(wlogon,&txtuser);
		draw_textbox(wlogon,&txtpasswd);
		draw_eyes(wlogon,&theeyes);
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
				quit=2;
			}

			result=update_button(wlogon,&btok,1,report->xbutton.x,report->xbutton.y);
			if(result){
				logon_pressok;
			}
		}
	}

	if(report->type==MotionNotify){
		cursor_textbox(wlogon,&txtuser,report->xmotion.x,report->xmotion.y);
		cursor_textbox(wlogon,&txtpasswd,report->xmotion.x,report->xmotion.y);
		cursor_button(wlogon,&btok,report->xmotion.x,report->xmotion.y);
		cursor_button(wlogon,&btabort,report->xmotion.x,report->xmotion.y);
	}

	if(report->type==KeyPress){
		logon_update_tab(report);

		key_button(wlogon,&btok,report,0);
		key_button(wlogon,&btabort,report,0);
		update_textbox(wlogon,&txtuser,report);
		update_textbox(wlogon,&txtpasswd,report);
		textbox_eyes(wlogon,&theeyes,&txtuser);
		textbox_eyes(wlogon,&theeyes,&txtpasswd);
	}
	
	if(report->type==KeyRelease){
		result=key_button(wlogon,&btabort,report,1);
		if(result){
			quit=2;
		}
		
		result=key_button(wlogon,&btok,report,1);
		if(result){
			logon_pressok;
		}
	}
	
	return;
}

void logon_update_tab(XEvent *report){
	if(XLookupKeysym(&(report->xkey),0)==65289){
		tab++;
		if(tab>WLOGON_MAXTAB){
			tab=1;
		}
		
		tab_button(wlogon,&btok);
		tab_button(wlogon,&btabort);
		tab_textbox(wlogon,&txtuser);
		tab_textbox(wlogon,&txtpasswd);
		
		button_eyes(wlogon,&theeyes,&btok);
		button_eyes(wlogon,&theeyes,&btabort);
	}
	return;
}


int logon_dialog(char *user,char *passwd){
	int td,pause=0;
	int posx,posy;
	int temp1;
	
	if(xopen()){
		printf("Connection to XServer couldn't be established.");
		return 1;
	}
	
	posx=WidthOfScreen(screeninfo);
	posy=HeightOfScreen(screeninfo);
	
	if((posx>2560)||((posx==2560)&&(posy==1024))){
		posx=((posx/4)-(WLOGON_WIDTH/2));
	}else{
		posx=((posx/2)-(WLOGON_WIDTH/2));
	}
	
	create_window(&wlogon,"Anmeldedaten eingeben","window",posx,((posy/2)-(WLOGON_HEIGHT/2)),WLOGON_WIDTH,WLOGON_HEIGHT);
	init_textbox(&txtuser,20,80,340,0,"Benutzername",0,1);
	init_textbox(&txtpasswd,20,110,340,0,"Passwort",1,2);
	init_button(&btok,"OK",110,160,60,0,1,3);
	init_button(&btabort,"Abbrechen",210,160,60,0,0,4);
	init_eyes(&theeyes,165,25,50,40);
	
	txtuser.state|=0x02;
	tab=txtuser.tabindex;
	
	
	if(user!=NULL){
		strcpy(txtuser.text,user);
		txtuser.len=strlen(user);
		
		left_scroll_textbox(&txtuser);
		txtuser.cursor=txtuser.scroll_end;
	}
	
	if(passwd!=NULL){
		strcpy(txtpasswd.text,passwd);
		txtpasswd.len=strlen(passwd);
		
		left_scroll_textbox(&txtpasswd);
		txtpasswd.cursor=txtpasswd.scroll_end;
	}

	quit=0;
	blink_count=0;
	temp1=0;
	gettimeofday(&st,NULL);
	while(!quit){
		td=time_diff();
		pause=delay(FRAME_LEN-td+pause); /*wait 25ms for saving cpu ticks*/
		
		if(blink_count>=20){
			blink_count=0;
			logon_blink_textboxes();
		}
		blink_count++;
		
		temp1++;
		if(temp1>=1200){
			return 3;
		}

		logon_eventloop();
	}

	strcpy(user,txtuser.text);
	strcpy(passwd,txtpasswd.text);
	
	ASCIItoUTF8(user);
	ASCIItoUTF8(passwd);
	
	close_window(wlogon);
	free_button(&btabort);
	free_button(&btok);
	free_textbox(&txtuser);
	free_textbox(&txtpasswd);

	xclose();

	if(quit==2){
		return 2;
	}

	return 0;
}









#define error_pressok quit=1

void error_eventloop(void){
	XEvent xev;
	int num_events;

	XFlush(display);
	num_events=XPending(display);
	while(num_events!=0){
		num_events--;
		
		XNextEvent(display,&xev);
		error_process_event(&xev);
	}
	return;
}

void error_process_event(XEvent *report){
	char result;
	
	if(report->type==ClientMessage){
		if(report->xclient.data.l[0]==wmDeleteMessage){
			quit=2;
		}
	}

	if(report->type==Expose){
		draw_button(werror,&btok);
		draw_text(werror,"Sie konnten nicht angemeldet werden.",10,15);
		draw_text(werror,"Wahrscheinlich ist Ihr Benutzername bzw. Kennwort ung\374ltig.",10,15+text_fontinfo->ascent+text_fontinfo->descent);
	}

	if(report->type==ButtonPress){
		if(report->xbutton.button==1){
			update_button(werror,&btok,0,report->xbutton.x,report->xbutton.y);
		}
	}

	if(report->type==ButtonRelease){
		if(report->xbutton.button==1){
			result=update_button(werror,&btok,1,report->xbutton.x,report->xbutton.y);
			if(result){
				error_pressok;
			}
		}
	}

	if(report->type==MotionNotify){
		cursor_button(werror,&btok,report->xmotion.x,report->xmotion.y);
	}

	if(report->type==KeyPress){
		error_update_tab(report);

		key_button(werror,&btok,report,0);
	}
	
	if(report->type==KeyRelease){
		result=key_button(werror,&btok,report,1);
		if(result){
			error_pressok;
		}
	}
	
	return;
}

void error_update_tab(XEvent *report){
	if(XLookupKeysym(&(report->xkey),0)==65289){
		tab++;
		if(tab>WERROR_MAXTAB){
			tab=1;
		}
		
		tab_button(werror,&btok);
	}
	return;
}

int logon_error(void){
	int td,pause=0;
	int posx,posy;

	if(xopen()){
		printf("Connection to XServer couldn't be established.");
		return 1;
	}
		
	posx=WidthOfScreen(screeninfo);
	posy=HeightOfScreen(screeninfo);
	
	if((posx>2560)||((posx==2560)&&(posy==1024))){
		posx=((posx/4)-(WERROR_WIDTH/2));
	}else{
		posx=((posx/2)-(WERROR_WIDTH/2));
	}
	
	create_window(&werror,"Anmeldung fehlgeschlagen","window",posx,((posy/2)-(WERROR_HEIGHT/2)),WERROR_WIDTH,WERROR_HEIGHT);
	init_button(&btok,"OK",WERROR_WIDTH-80,50,60,0,1,1);
	
	btok.state|=0x04;
	
	quit=0;
	gettimeofday(&st,NULL);
	while(!quit){
		td=time_diff();
		pause=delay(FRAME_LEN-td+pause);
		error_eventloop();
	}
	
	close_window(werror);
	free_button(&btok);
	
	xclose();
	
	return 0;
}










#define cert_pressyes quit=1

void cert_eventloop(int buttons){
	XEvent xev;
	int num_events;

	XFlush(display);
	num_events=XPending(display);
	while(num_events!=0){
		num_events--;
		
		XNextEvent(display,&xev);
		cert_process_event(&xev,buttons);
	}
	return;
}

void cert_process_event(XEvent *report,int buttons){
	char result;
	
	if(report->type==ClientMessage){
		if(report->xclient.data.l[0]==wmDeleteMessage){
			quit=2;
		}
	}

	if(report->type==Expose){
		draw_button(wcert,&btyes);
		if(buttons)
			draw_button(wcert,&btno);
		draw_itext(wcert,cert_text,10,15,WCERT_WIDTH-10);
	}

	if(report->type==ButtonPress){
		if(report->xbutton.button==1){
			update_button(wcert,&btyes,0,report->xbutton.x,report->xbutton.y);
			if(buttons)
				update_button(wcert,&btno,0,report->xbutton.x,report->xbutton.y);
		}
	}

	if(report->type==ButtonRelease){
		if(report->xbutton.button==1){
			result=update_button(wcert,&btyes,1,report->xbutton.x,report->xbutton.y);
			if(result){
				cert_pressyes;
			}
		}
		
		if(report->xbutton.button==1){
			if(buttons){
				result=update_button(wcert,&btno,1,report->xbutton.x,report->xbutton.y);
				if(result){
					quit=2;
				}
			}
		}
	}

	if(report->type==MotionNotify){
		cursor_button(wcert,&btyes,report->xmotion.x,report->xmotion.y);
		if(buttons)
			cursor_button(wcert,&btno,report->xmotion.x,report->xmotion.y);
	}

	if(report->type==KeyPress){
		cert_update_tab(report,buttons);

		key_button(wcert,&btyes,report,0);
		if(buttons)
			key_button(wcert,&btno,report,0);
	}
	
	if(report->type==KeyRelease){
		result=key_button(wcert,&btyes,report,1);
		if(result){
			cert_pressyes;
		}
		
		if(buttons){
			result=key_button(wcert,&btno,report,1);
			if(result){
				quit=2;
			}
		}
	}
	
	return;
}

void cert_update_tab(XEvent *report,int buttons){
	if(XLookupKeysym(&(report->xkey),0)==65289){
		tab++;
		if(tab>WCERT_MAXTAB){
			tab=1;
		}
		
		tab_button(wcert,&btyes);
		if(buttons)
			tab_button(wcert,&btno);
	}
	return;
}

int cert_error(char *value,int buttons){
	int td,pause=0;
	int posx,posy;
	
	cert_text=value;

	if(xopen()){
		printf("Connection to XServer couldn't be established.");
		return 1;
	}
		
	posx=WidthOfScreen(screeninfo);
	posy=HeightOfScreen(screeninfo);
	
	if((posx>2560)||((posx==2560)&&(posy==1024))){
		posx=((posx/4)-(WCERT_WIDTH/2));
	}else{
		posx=((posx/2)-(WCERT_WIDTH/2));
	}
	
	create_window(&wcert,"certificate missmatch","window",posx,((posy/2)-(WCERT_HEIGHT/2)),WCERT_WIDTH,WCERT_HEIGHT);
	if(buttons){
		init_button(&btyes,"yes",195,WCERT_HEIGHT-40,60,0,0,1);
		init_button(&btno,"no",295,WCERT_HEIGHT-40,60,0,0,2);
		
		btno.state|=0x04;
	}else{
		init_button(&btyes,"OK",245,WCERT_HEIGHT-40,60,0,0,1);
	}
	
	tab=2;
	
	quit=0;
	gettimeofday(&st,NULL);
	while(!quit){
		td=time_diff();
		pause=delay(FRAME_LEN-td+pause);
		cert_eventloop(buttons);
	}
	
	close_window(wcert);
	free_button(&btyes);
	if(buttons)
		free_button(&btno);
	
	xclose();
	
	if(quit==2){
		return 2;
	}
	
	return 0;
}

#include "X-tools.c"

#endif
/** 
 * (Some) useful functions to calculate time delay(s), such as used
 * with eventloop(s) for querying Xevents using Xlib.
 * 
 * Copyright 2014 Thomas Erbesdobler <t.erbesdobler@team103.de>
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
 * NOTICE
 * 	This code was written using the example delay functions shown in
 * 	the book "Linux-UNIX-programmierung" (subtitel: "Das umfassende Handbuch"),
 * 	2nd edition, by Jürgen Wolf at the end of chapter 14.4.
 */

#ifndef DELAY_H
#define DELAY_H

#include <sys/time.h>
#define FRAME_LEN 25000

static struct timeval st,rt;
static int delay(int i){
	struct timeval timeout;
	if(i>0){
		timeout.tv_usec=i%(unsigned long)1000000;
		timeout.tv_sec=i/(unsigned long)1000000;
		select(0,NULL,NULL,NULL,&timeout);
	}
	return (i>0 ? i : 0);
}
static int time_diff(void){
	int diff;
	gettimeofday(&rt,NULL);
	diff=(1000000*(rt.tv_sec-st.tv_sec))+(rt.tv_usec-st.tv_usec);
	st=rt;
	return diff;
}

#endif

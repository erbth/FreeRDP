/**
 * Copyright 2015 Thomas Erbesdobler <t.erbesdobler@team103.com>
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
 * This file is actually the same as prim_YUV.h for converting IYUV coded
 * images to RGB, created by and under the Copyright of Marc-Andre Moreau
 * <marcandre.moreau@gmail.com> 2014 as part of FreeRDP.
 *
 * Because NV12 is just another YUV format, the conversation is the same
 * and getting data is just slightly different.
 */

#ifndef FREERDP_PRIMITIVES_NV12_H
#define FREERDP_PRIMITIVES_NV12_H

pstatus_t general_NV12ToRGB_8u_P2AC4R(const BYTE* pSrc[2], int srcStep[2], BYTE* pDst, int dstStep, const prim_size_t* roi);

void primitives_init_NV12(primitives_t* prims);
void primitives_init_NV12_opt(primitives_t* prims);
void primitives_deinit_NV12(primitives_t* prims);

#endif /* FREERDP_PRIMITIVES_YUV_H */

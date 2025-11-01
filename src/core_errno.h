/* 
* Copyright 1998-2014 VIA Technologies, Inc. All Rights Reserved.
* Copyright 2001-2014 S3 Graphics, Inc. All Rights Reserved.
* Copyright 2013-2014 Shanghai Zhaoxin Semiconductor Co., Ltd. All Rights Reserved.
*
* This file is part of zx.ko
* 
* zx.ko is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* zx.ko is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __ERRNO_H_
#define __ERRNO_H_

#define S_OK                0x00000000
#define E_OUTOFMEMORY       0x80000002
#define E_NOIMPL            0x80004001
#define E_INVALIDARG        0x80070057
#define E_NOINTERFACE       0x80004002
#define E_POINTER           0x80004003
#define E_UNEXPECTED        0x8000FFFF
#define E_FAIL              0x10004005
#define E_UNSWIZZLING_APERTURE_UNSUPPORTED  0x10004006
/*page out allocation but allocation still used by GPU hw*/
#define E_PAGEOUT_ALLOCATION_BUSY  0x10004007
#define E_INSUFFICIENT_DMA_BUFFER   0xFFFF0001


#endif


/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef __HPHP_TAINTING_H__
#define __HPHP_TAINTING_H__

#ifdef TAINTED

/* The tainting library consists of three main parts:
 * - booleans in StringData and StringBuffer allow to know whether a string
 *   is tainted or not.
 * - TaintedMetadata (in tainted_metadata.h / .cpp) allows to store
 *   metadata about the tainting, such as where it was tainted, how
 *   it was concatenated, etc.
 * - This file allows to easily update and transfer taints and tainting
 *   metadata.
 */

#include <runtime/base/types.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/util/string_buffer.h>

namespace HPHP {
bool propagate_tainting_aux(CStrRef orig, String& dest);
/* 6 variants to avoid repetition:
 * propagate_tainting<i> propagates the tainting to dest whenever one of the
 * orig<j> is tainted; only the metadata and the tainting from the first
 * one which was tainted is kept
 */
void propagate_tainting1(CStrRef orig, String& dest);
void propagate_tainting2(CStrRef orig1, CStrRef orig2,
                           String& dest);
void propagate_tainting3(CStrRef orig1, CStrRef orig2,
                           CStrRef orig3,
                           String& dest);
void propagate_tainting4(CStrRef orig1, CStrRef orig2,
                           CStrRef orig3, CStrRef orig4,
                           String& dest);
void propagate_tainting5(CStrRef orig1, CStrRef orig2,
                           CStrRef orig3, CStrRef orig4,
                           CStrRef orig5,
                           String& dest);
void propagate_tainting6(CStrRef orig1, CStrRef orig2,
                           CStrRef orig3, CStrRef orig4,
                           CStrRef orig5, CStrRef orig6,
                           String& dest);

void propagate_tainting2_buf(CStrRef orig1, StringBuffer const &orig2,
                                            StringBuffer& dest);
void propagate_tainting1_buf(StringBuffer const &orig, String& dest);
void propagate_tainting1_bufbuf(StringBuffer const &orig, StringBuffer& dest);
}

#endif

#endif // __HPHP_TAINTING_H__

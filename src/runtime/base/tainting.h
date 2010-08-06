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

typedef int bitstring;

const bitstring default_tainting = 0x00; // 0000 TODOjjeannin
const bitstring tainting_bit_metadata = 0x01;
inline bool is_tainting_metadata(bitstring b){
  if(b & tainting_bit_metadata){ return true; }
  else { return false; }
}

// TODOjjeannin: for now this has to be the same as tainting_bit_metadata
// otherwise need to change ExecutionContext::write(CStrRef s) in
// src/runtime/base/execution_context.cpp
const bitstring tainting_bit_html = 0x01;
inline bool is_tainted_html(bitstring b){
  if(b & tainting_bit_html){ return true; }
  else { return false; }
}

#endif

#endif // __HPHP_TAINTING_H__

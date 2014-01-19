/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/verifier/util.h"

#include <stdlib.h>
#include <string.h>

namespace HPHP {  namespace Verifier {

Bits::Bits(Arena& arena, int bit_cap) {
  int word_cap = (bit_cap + BPW - 1) / BPW;
  m_words = new (arena) uintptr_t[word_cap];
  memset(m_words, 0, word_cap * sizeof(*m_words));
}

}}

/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifdef TAINTED

#include <runtime/base/taint/taint_data.h>

namespace HPHP {

TaintData::TaintData() {
  m_taint_bits = TAINT_BIT_NONE;
  m_metadata = NULL;
}

bitstring TaintData::getTaint() const {
  return m_taint_bits;
}

void TaintData::setTaint(bitstring bits) {
  m_taint_bits = m_taint_bits | bits;
}

void TaintData::unsetTaint(bitstring bits) {
  m_taint_bits = m_taint_bits & (~bits);
}

void TaintData::dump() const {
  printf("Taint: %x\n", m_taint_bits);
}

}

#endif // TAINTED

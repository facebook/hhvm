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
}

bitstring TaintData::getTaint() const {
  return m_taint_bits;
}

void TaintData::setTaint(bitstring bits, const char* original_str) {
  m_taint_bits = m_taint_bits | bits;

  if ((bits & TAINT_BIT_HTML) && !m_metadata.get()) {
    // Set the metadata if TAINT_BIT_HTML is being set
    // Because we don't backup strings, we can end up in situtations where
    // a string for not have any metadata. That is fine for now.
    if (original_str) {
      m_metadata = TaintMetadataPtr(new TaintMetadata(original_str));
    }
  }
}

void TaintData::unsetTaint(bitstring bits) {
  m_taint_bits = m_taint_bits & (~bits);

  if ((bits & TAINT_BIT_HTML) && m_metadata.get()) {
    // Remove the metadata if TAINT_BIT_HTML is being unset
    m_metadata.reset();
  }
}

const char* TaintData::getOriginalStr() const {
  if (m_metadata.get()) {
    return m_metadata->getOriginalStr();
  } else {
    return NULL;
  }
}

void TaintData::clearMetadata() {
  m_metadata.reset();
}

void TaintData::dump() const {
  printf("Taint: %x\n", m_taint_bits);
  if (m_metadata.get()) {
    printf("Original string: %s\n", m_metadata.get()->getOriginalStr());
    printf("Count: %d", m_metadata.get()->getCount());
  } else {
    printf("Original string: n/a");
  }
}

}

#endif // TAINTED

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

#ifdef TAINTED

#include <runtime/base/taint.h>
#include <runtime/base/util/string_buffer.h>
#include <runtime/base/tainted_metadata.h>

/**
 * This class makes it easier to taint things.
 *
 * You can do things like:
 * Taint(a) << b << c;
 *
 * HPHP has the following types of strings
 * - String (which is an indirection to StringData)
 * - Variant (which can be an indirection to StringData or another data type)
 * - StringData
 * - StringBuffer (used internally)
 *
 * The taint bits only exists on StringData and StringBuffer.
 */

using namespace std;

namespace HPHP {

Taint::Taint(StringBuffer &str) {
  m_bits = str.getTaintBitString();
  m_metadata = str.getTaintMetaData();
}

Taint& Taint::operator <<(const String &src) {
  bitstring src_bits = src.getTaint();

  *m_bits = *m_bits | src_bits;

  if (is_tainting_metadata(src_bits)) {
    propagateMetaData(src.getTaintedMetadata());
  }

  return *this;
}

Taint& Taint::operator <<(const StringData &src) {
  bitstring src_bits = src.getTaint();

  *m_bits = *m_bits | src_bits;

  if (is_tainting_metadata(src_bits)) {
    propagateMetaData(src.getTaintedMetadata());
  }

  return *this;
}

void Taint::propagateMetaData(TaintedMetadata* metadata) {
  if (*m_metadata) {
    // we already have some metadata, so we need to replace it
    delete *m_metadata;
  }
  *m_metadata = new TaintedMetadata();

  (*m_metadata)->setTaintedOriginal(metadata->getTaintedOriginal());
  (*m_metadata)->setTaintedPlace(*(metadata->getTaintedPlace()));
  (*m_metadata)->setTaintedChanged(metadata->getTaintedChanged());
  (*m_metadata)->addTaintedChanged();
}

}

#endif // TAINTED

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

#include <runtime/base/taint/taint_metadata.h>
#include <runtime/base/frame_injection.h>
#include <runtime/base/util/extended_logger.h>

namespace HPHP {

TaintMetadata::TaintMetadata(const char* original_str) {
  int l = strlen(original_str);
  m_original_str = (char*)malloc(l+1);
  strncpy(m_original_str, original_str, l);
  m_original_str[l] = 0;
}

const char* TaintMetadata::getOriginalStr() const {
  return m_original_str;
}

void TaintMetadata::release() {
  free(m_original_str);
  m_original_str = NULL;
  delete this;
}

}

#endif // TAINTED

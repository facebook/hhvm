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

#ifndef __HPHP_TAINT_H__
#define __HPHP_TAINT_H__

#ifdef TAINTED

#include <runtime/base/util/string_buffer.h>
#include <runtime/base/tainting.h>
#include <runtime/base/tainted_metadata.h>

/**
 * This class handles the tainting of strings.
 *
 * Initially the code was duplicated in several places, so this class makes
 * everything cleaner.
 */
using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class Taint {
public:
  Taint(StringBuffer &str);
  Taint& operator<<(const String& src);
  Taint& operator<<(const StringData& src);

private:
  void propagateMetaData(TaintedMetadata* metadata);

  bitstring *m_bits;
  TaintedMetadata **m_metadata;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // TAINTED

#endif // __HPHP_TAINT_H__

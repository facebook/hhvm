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

#ifndef __HPHP_TAINT_DATA_H__
#define __HPHP_TAINT_DATA_H__

#ifdef TAINTED

#define TAINT_BIT_HTML (0x01)
#define TAINT_BIT_SQL  (0x02)
#define TAINT_BIT_ALL  (0x03)
#define TAINT_BIT_NONE (0x00)

#include <runtime/base/taint/taint_metadata.h>

namespace HPHP {

typedef int bitstring;

class TaintData {
public:
  TaintData();
  bitstring getTaint() const;
  void setTaint(bitstring bits, const char* original_str);
  void unsetTaint(bitstring bits);
  const char* getOriginalStr() const;

  void clearMetadata();

  void dump() const;
private:
  bitstring m_taint_bits;
  TaintMetadataPtr m_metadata;
};

}

#endif // TAINTED

#endif // __HPHP_TAINT_DATA_H__

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

#ifndef __HPHP_TAINT_METADATA_H__
#define __HPHP_TAINT_METADATA_H__

#ifdef TAINTED

#include <runtime/base/util/countable.h>
#include <runtime/base/util/smart_ptr.h>

namespace HPHP {

class TaintMetadata : public Countable {
public:
  TaintMetadata(const char* original_str);
  const char* getOriginalStr() const;
  void release();
private:
  char* m_original_str;
};

class TaintMetadataPtr : public SmartPtr<TaintMetadata> {
public:
  TaintMetadataPtr() {}
  TaintMetadataPtr(TaintMetadata *px) : SmartPtr<TaintMetadata>(px) {}
};

}

#endif // TAINTED

#endif // __HPHP_TAINT_DATA_H__

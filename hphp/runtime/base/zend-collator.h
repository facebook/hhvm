/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_ZEND_COLLATOR_H_
#define incl_HPHP_ZEND_COLLATOR_H_

#include "hphp/runtime/base/complex_types.h"
#include "hphp/runtime/base/request_local.h"
#include "unicode/coll.h" // icu

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct intl_error {
  UErrorCode code;
  String custom_error_message;
  intl_error() : code(U_ZERO_ERROR) {}
  void clear() {
    code = U_ZERO_ERROR;
    custom_error_message.reset();
  }
};

class IntlError : public RequestEventHandler {
public:
  intl_error m_error;
  IntlError() {
    m_error.clear();
  }
  virtual void requestInit() {
    m_error.clear();
  }
  virtual void requestShutdown() {
    m_error.clear();
  }
};

DECLARE_EXTERN_REQUEST_LOCAL(IntlError, s_intl_error);

///////////////////////////////////////////////////////////////////////////////

#define COLLATOR_SORT_REGULAR   0
#define COLLATOR_SORT_STRING    1
#define COLLATOR_SORT_NUMERIC   2

bool collator_sort(Variant &array, int sort_flags, bool ascending,
                   UCollator *coll, intl_error *errcode);
bool collator_asort(Variant &array, int sort_flags, bool ascending,
                    UCollator *coll, intl_error *errcode);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_ZEND_COLLATOR_H_

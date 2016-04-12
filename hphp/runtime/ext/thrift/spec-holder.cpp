/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/ext/thrift/spec-holder.h"

#include "hphp/runtime/base/array-init.h"

namespace HPHP { namespace thrift {

void thrift_error(const String& what, TError why) {
  throw_object(s_TProtocolException, make_packed_array(what, why));
}

Array get_tspec(const Class* cls) {
  /*
    passing in cls will short-circuit the accessibility checks,
    but does mean we'll allow a private or protected s_TSPEC.
    passing in nullptr would do the correct checks. Not sure it matters
  */
  auto lookup = cls->getSProp(cls, s_TSPEC.get());
  if (!lookup.prop) {
    thrift_error(
      folly::sformat("Class {} does not have a property named {}",
                     cls->name(), s_TSPEC),
      ERR_INVALID_DATA);
  }
  Variant structSpec = tvAsVariant(lookup.prop);
  if (!structSpec.isArray()) {
    thrift_error("invalid type of spec", ERR_INVALID_DATA);
  }
  return structSpec.toArray();
}

SpecCacheMap SpecHolder::s_specCacheMap(1000);

}}

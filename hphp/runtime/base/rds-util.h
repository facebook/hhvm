/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#ifndef incl_HPHP_RUNTIME_BASE_RDS_UTIL_H_
#define incl_HPHP_RUNTIME_BASE_RDS_UTIL_H_

#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/typed-value.h"

namespace HPHP {
  struct NamedEntity;
  struct Class;
  struct Func;
  struct StringData;
  struct MemoCacheBase;
}

namespace HPHP { namespace rds {

//////////////////////////////////////////////////////////////////////

/*
 * Utility functions for allocating some types of data from RDS.
 */

/*
 * Allocate storage for the value of a class constant in RDS.
 */
Link<TypedValue, rds::Mode::Normal>
bindClassConstant(const StringData* clsName, const StringData* cnsName);

Link<TypedValue, rds::Mode::Normal>
bindStaticMemoValue(const Func*);

Link<MemoCacheBase*, rds::Mode::Normal>
bindStaticMemoCache(const Func*);

Link<TypedValue, rds::Mode::Normal>
attachStaticMemoValue(const Func*);

Link<MemoCacheBase*, rds::Mode::Normal>
attachStaticMemoCache(const Func*);

Link<TypedValue, rds::Mode::Normal>
bindLSBMemoValue(const Class*, const Func*);

Link<MemoCacheBase*, rds::Mode::Normal>
bindLSBMemoCache(const Class*, const Func*);

Link<TypedValue, rds::Mode::Normal>
attachLSBMemoValue(const Class*, const Func*);

Link<MemoCacheBase*, rds::Mode::Normal>
attachLSBMemoCache(const Class*, const Func*);

//////////////////////////////////////////////////////////////////////

}}

#endif

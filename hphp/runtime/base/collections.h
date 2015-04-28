/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_COLLECTIONS_H_
#define incl_HPHP_COLLECTIONS_H_

#include <folly/Optional.h>

#include "hphp/runtime/base/header-kind.h"
#include "hphp/runtime/base/type-string.h"

namespace HPHP {

/*
 * Returns a collection type name given a CollectionType.
 */
StringData* collectionTypeToString(CollectionType ctype);

/*
 * Returns a CollectionType given a name, folly::none if name is not a
 * collection type.
 */
folly::Optional<CollectionType> stringToCollectionType(const StringData* name);

inline folly::Optional<CollectionType>
stringToCollectionType(const std::string& s) {
  return stringToCollectionType(StringData::Make(s.c_str()));
}

inline bool isCollectionTypeName(const StringData* str) {
  return stringToCollectionType(str).hasValue();
}

}

#endif

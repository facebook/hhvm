/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/collections.h"

namespace HPHP {

const StaticString
  s_Vector("HH\\Vector"),
  s_Map("HH\\Map"),
  s_Set("HH\\Set"),
  s_Pair("HH\\Pair"),
  s_ImmVector("HH\\ImmVector"),
  s_ImmSet("HH\\ImmSet"),
  s_ImmMap("HH\\ImmMap");

StringData* collectionTypeToString(CollectionType ctype) {
  switch (ctype) {
    case CollectionType::Vector:
      return s_Vector.get();
    case CollectionType::Map:
      return s_Map.get();
    case CollectionType::Set:
      return s_Set.get();
    case CollectionType::Pair:
      return s_Pair.get();
    case CollectionType::ImmVector:
      return s_ImmVector.get();
    case CollectionType::ImmSet:
      return s_ImmSet.get();
    case CollectionType::ImmMap:
      return s_ImmMap.get();
  }
  assert(!"Unknown Collection Type");
  not_reached();
}

folly::Optional<CollectionType> stringToCollectionType(const StringData* name) {
  switch (name->size()) {
    case 6:
      if (name->isame(s_Set.get())) return CollectionType::Set;
      if (name->isame(s_Map.get())) return CollectionType::Map;
      break;
    case 7:
      if (name->isame(s_Pair.get())) return CollectionType::Pair;
      break;
    case 9:
      if (name->isame(s_Vector.get())) return CollectionType::Vector;
      if (name->isame(s_ImmMap.get())) return CollectionType::ImmMap;
      if (name->isame(s_ImmSet.get())) return CollectionType::ImmSet;
      break;
    case 12:
      if (name->isame(s_ImmVector.get())) return CollectionType::ImmVector;
      break;
    default:
      break;
  }
  return folly::none;
}

}

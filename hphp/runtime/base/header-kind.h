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

#ifndef incl_HPHP_HEADER_KIND_H_
#define incl_HPHP_HEADER_KIND_H_

namespace HPHP {

enum class HeaderKind : uint8_t {
  // ArrayKind aliases
  Packed, Struct, Mixed, Empty, Apc, Globals, Proxy,
  // Other ordinary refcounted heap objects
  String, Resource, Ref,
  Object, ResumableObj, AwaitAllWH,
  Vector, Map, Set, Pair, ImmVector, ImmMap, ImmSet,
  Resumable, // ResumableNode followed by Frame, Resumable, ObjectData
  Native, // a NativeData header preceding an HNI ObjectData
  SmallMalloc, // small smart_malloc'd block
  BigMalloc, // big smart_malloc'd block
  BigObj, // big size-tracked object (valid header follows BigNode)
  Free, // small block in a FreeList
  Hole, // wasted space not in any freelist
  Debug // a DebugHeader
};

const size_t HeaderKindOffset = 11;
const unsigned NumHeaderKinds = unsigned(HeaderKind::Debug) + 1;

inline bool isObjectKind(HeaderKind k) {
  return uint8_t(k) >= uint8_t(HeaderKind::Object) &&
         uint8_t(k) <= uint8_t(HeaderKind::ImmSet);
}

namespace Collection {

enum Type : uint8_t { // Subset of possible HeaderKind values
  // Values must be contiguous integers (for ArrayIter::initFuncTable).
  InvalidType = static_cast<Type>(HeaderKind::AwaitAllWH),
  VectorType = static_cast<Type>(HeaderKind::Vector),
  MapType = static_cast<Type>(HeaderKind::Map),
  SetType = static_cast<Type>(HeaderKind::Set),
  PairType = static_cast<Type>(HeaderKind::Pair),
  ImmVectorType = static_cast<Type>(HeaderKind::ImmVector),
  ImmMapType = static_cast<Type>(HeaderKind::ImmMap),
  ImmSetType = static_cast<Type>(HeaderKind::ImmSet),
};

constexpr size_t MaxNumTypes = 8;

inline bool isVectorType(Type ctype) {
  return ctype == VectorType || ctype == ImmVectorType;
}
inline bool isMapType(Type ctype) {
  return ctype == MapType || ctype == ImmMapType;
}
inline bool isSetType(Type ctype) {
  return ctype == SetType || ctype == ImmSetType;
}
inline bool isValidType(Type ctype) {
  return static_cast<uint8_t>(ctype) >= VectorType &&
         static_cast<uint8_t>(ctype) <= ImmSetType;
}
inline bool isMutableType(Type ctype) {
  return ctype == VectorType || ctype == MapType || ctype == SetType;
}
inline bool isImmutableType(Type ctype) {
  return !isMutableType(ctype);
}

inline bool isTypeWithPossibleIntStringKeys(Type ctype) {
  return isSetType(ctype) || isMapType(ctype);
}

}
}

#endif

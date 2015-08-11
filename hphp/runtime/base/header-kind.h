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
  ResumableFrame, // ResumableNode followed by Frame, Resumable, ObjectData
  NativeData, // a NativeData header preceding an HNI ObjectData
  SmallMalloc, // small req::malloc'd block
  BigMalloc, // big req::malloc'd block
  BigObj, // big size-tracked object (valid header follows BigNode)
  Free, // small block in a FreeList
  Hole, // wasted space not in any freelist
};
const unsigned NumHeaderKinds = unsigned(HeaderKind::Hole) + 1;
extern const char* header_names[];

/*
 * RefCount type for m_count field in refcounted objects
 */
using RefCount = int32_t;
using GCByte = uint8_t;

/*
 * Common header for all heap-allocated objects. Layout is carefully
 * designed to allow overlapping with the second word of a TypedValue,
 * or to follow a C++ defined vptr.
 */
template<class T = uint16_t> struct HeaderWord {
  union {
    struct {
      union {
        struct {
          T aux;
          HeaderKind kind;
          union {
            struct {
              mutable uint8_t mark:1;
              mutable uint8_t cmark:1;
              mutable uint8_t mrb:1;
              uint8_t spare:3;
              mutable uint8_t uncounted:1;
              mutable uint8_t _static:1;
            };
            mutable GCByte gcbyte;
          };
        };
        uint32_t lo32;
      };
      union {
        mutable RefCount count;
        uint32_t hi32;
      };
    };
    uint64_t q;
  };

  void init(HeaderKind kind, GCByte gc) {
    q = static_cast<uint32_t>(kind) << (8 * offsetof(HeaderWord, kind)) |
        gc << (8 * offsetof(HeaderWord, gcbyte));
  }

  void init(T aux, HeaderKind kind, GCByte gc) {
    q = static_cast<uint32_t>(kind) << (8 * offsetof(HeaderWord, kind)) |
        static_cast<uint16_t>(aux) |
        gc << (8 * offsetof(HeaderWord, gcbyte));
    static_assert(sizeof(T) == 2, "header layout requres 2-byte aux");
  }

  void init(const HeaderWord<T>& h, GCByte gc) {
    q = static_cast<uint32_t>(h.kind) << (8 * offsetof(HeaderWord, kind)) |
        static_cast<uint16_t>(h.aux) |
        gc << (8 * offsetof(HeaderWord, gcbyte));
  }
};

constexpr auto HeaderOffset = sizeof(void*);
constexpr auto HeaderKindOffset = HeaderOffset + offsetof(HeaderWord<>, kind);
constexpr auto FAST_REFCOUNT_OFFSET = HeaderOffset +
                                      offsetof(HeaderWord<>, count);
constexpr auto FAST_GC_BYTE_OFFSET = HeaderOffset +
                                      offsetof(HeaderWord<>, gcbyte);
constexpr int8_t FAST_MRB_MASK = 1 << 2;
constexpr int8_t FAST_UNCOUNTED_MASK = 1 << 6;

inline bool isObjectKind(HeaderKind k) {
  return k >= HeaderKind::Object && k <= HeaderKind::ImmSet;
}

inline bool isArrayKind(HeaderKind k) {
  return k >= HeaderKind::Packed && k <= HeaderKind::Proxy;
}

enum class CollectionType : uint8_t { // Subset of possible HeaderKind values
  // Values must be contiguous integers (for ArrayIter::initFuncTable).
  Vector = uint8_t(HeaderKind::Vector),
  Map = uint8_t(HeaderKind::Map),
  Set = uint8_t(HeaderKind::Set),
  Pair = uint8_t(HeaderKind::Pair),
  ImmVector = uint8_t(HeaderKind::ImmVector),
  ImmMap = uint8_t(HeaderKind::ImmMap),
  ImmSet = uint8_t(HeaderKind::ImmSet),
};

inline bool isVectorCollection(CollectionType ctype) {
  return ctype == CollectionType::Vector || ctype == CollectionType::ImmVector;
}
inline bool isMapCollection(CollectionType ctype) {
  return ctype == CollectionType::Map || ctype == CollectionType::ImmMap;
}
inline bool isSetCollection(CollectionType ctype) {
  return ctype == CollectionType::Set || ctype == CollectionType::ImmSet;
}
inline bool isValidCollection(CollectionType ctype) {
  return uint8_t(ctype) >= uint8_t(CollectionType::Vector) &&
         uint8_t(ctype) <= uint8_t(CollectionType::ImmSet);
}
inline bool isMutableCollection(CollectionType ctype) {
  return ctype == CollectionType::Vector ||
         ctype == CollectionType::Map ||
         ctype == CollectionType::Set;
}
inline bool isImmutableCollection(CollectionType ctype) {
  return !isMutableCollection(ctype);
}

inline bool collectionAllowsIntStringKeys(CollectionType ctype) {
  return isSetCollection(ctype) || isMapCollection(ctype);
}

}

#endif

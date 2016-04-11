#include "hphp/runtime/ext/collections/ext_collections.h"
#include "hphp/runtime/ext/collections/ext_collections-map.h"
#include "hphp/runtime/ext/collections/ext_collections-set.h"
#include "hphp/runtime/ext/collections/ext_collections-pair.h"
#include "hphp/runtime/ext/collections/ext_collections-vector.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/memory-manager.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

void triggerCow(c_Vector* vec) {
  vec->mutateImpl();
}

ArrayIter getArrayIterHelper(const Variant& v, size_t& sz) {
  if (v.isArray()) {
    ArrayData* ad = v.getArrayData();
    sz = ad->size();
    return ArrayIter(ad);
  }
  if (v.isObject()) {
    ObjectData* obj = v.getObjectData();
    if (obj->isCollection()) {
      sz = collections::getSize(obj);
      return ArrayIter(obj);
    }
    bool isIterable;
    Object iterable = obj->iterableObject(isIterable);
    if (isIterable) {
      sz = 0;
      return ArrayIter(iterable.detach(), ArrayIter::noInc);
    }
  }
  SystemLib::throwInvalidArgumentExceptionObject(
    "Parameter must be an array or an instance of Traversable");
}

namespace collections {
/////////////////////////////////////////////////////////////////////////////

size_t heapSize(HeaderKind kind) {
  switch (kind) {
#define X(T) case HeaderKind::T: \
               return sizeof(c_##T);
COLLECTIONS_ALL_TYPES(X)
#undef X
    default: always_assert(false);
  }
}

/////////////////////////////////////////////////////////////////////////////

template<bool oob>
[[noreturn]] ALWAYS_INLINE
void throwInt(int64_t key) {
  String msg(50, ReserveString);
  auto buf = msg.bufferSlice();
  auto sz = snprintf(
    buf.data(), buf.size() + 1,
    "Integer key %" PRId64 " is %s", key,
    oob ? "out of bounds" : "not defined"
  );
  msg.setSize(std::min<int>(sz, buf.size()));
  SystemLib::throwOutOfBoundsExceptionObject(msg);
}


void throwOOB(int64_t key)   { throwInt<true>(key); }
void throwUndef(int64_t key) { throwInt<false>(key); }

void throwUndef(const StringData* key) {
  constexpr size_t maxDisplaySize = 100;
  auto const keySize = key->size();
  auto const keyIsLarge = (keySize > maxDisplaySize);
  folly::StringPiece part1{"String key \""};
  folly::StringPiece part3 = keyIsLarge ? "\" (truncated) is not defined" :
                                   "\" is not defined";
  folly::StringPiece part2(key->data(), keyIsLarge ? maxDisplaySize : keySize);
  String msg(part1.size() + part2.size() + part2.size(), ReserveString);
  msg += part1;
  msg += part2;
  msg += part3;
  SystemLib::throwOutOfBoundsExceptionObject(msg);
}

/////////////////////////////////////////////////////////////////////////////
static CollectionsExtension s_collections_extension;
/////////////////////////////////////////////////////////////////////////////
}}

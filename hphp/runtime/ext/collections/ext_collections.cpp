#include "hphp/runtime/ext/collections/ext_collections.h"
#include "hphp/runtime/ext/collections/ext_collections-idl.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/memory-manager.h"

namespace HPHP { namespace collections {
/////////////////////////////////////////////////////////////////////////////

const StaticString
  s_HH_Vector("HH\\Vector"),
  s_HH_ImmVector("HH\\ImmVector"),
  s_HH_Map("HH\\Map"),
  s_HH_ImmMap("HH\\ImmMap"),
  s_HH_Set("HH\\Set"),
  s_HH_ImmSet("HH\\ImmSet");

void instanceDtor(ObjectData* obj, const Class* cls) {
  assert(obj->getAttribute(ObjectData::IsCollection));
  assert(cls->isCollectionClass());
  assert(cls->numDeclProperties() == 0);
  assert(cls->attrs() & AttrFinal);

  switch (obj->headerKind()) {
#define X(knd) case HeaderKind::knd: \
                 (static_cast<c_##knd*>(obj))->~c_##knd(); \
                 MM().objFree(obj, sizeof(c_##knd)); \
                 break;
COLLECTIONS_ALL_TYPES(X)
#undef X
    default: always_assert(false);
  }
}

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
ALWAYS_INLINE ATTRIBUTE_NORETURN
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

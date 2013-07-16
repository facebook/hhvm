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

#ifndef incl_HPHP_ARRAY_DATA_H_
#define incl_HPHP_ARRAY_DATA_H_

#include "hphp/runtime/base/countable.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/macros.h"
#include <climits>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class SharedVariant;
struct TypedValue;
class HphpArray;

/**
 * Base class/interface for all types of specialized array data.
 */
class ArrayData {
 public:
  enum class AllocationMode : bool { smart, nonSmart };

  // enum of possible array types, so we can guard nonvirtual
  // fast paths in runtime code.  This is intentionally not
  // an enum class, to avoid boilerplate when:
  //  - doing relational comparisons
  //  - using kind as an index
  //  - maybe doing bitops in the future
  enum ArrayKind : uint8_t {
    kVectorKind,  // HphpArray vector-shape
    kMixedKind,   // HphpArray generic shape
    kSharedKind,  // SharedMap
    kNvtwKind,    // NameValueTableWrapper
    kPolicyKind,  // PolicyArray
    kNumKinds // insert new values before kNumKinds.
  };

public:
  static const ssize_t invalid_index = -1;

  explicit ArrayData(ArrayKind kind)
    : m_kind(kind)
    , m_allocMode(AllocationMode::smart)
    , m_size(-1)
    , _count(0)
    , m_pos(0)
    , m_strongIterators(nullptr)
  {}

  explicit ArrayData(ArrayKind kind, AllocationMode m)
    : m_kind(kind)
    , m_allocMode(m)
    , m_size(-1)
    , _count(0)
    , m_pos(0)
    , m_strongIterators(nullptr)
  {}

  ArrayData(ArrayKind kind, AllocationMode m, uint size)
    : m_kind(kind)
    , m_allocMode(m)
    , m_size(size)
    , _count(0)
    , m_pos(size ? 0 : ArrayData::invalid_index)
    , m_strongIterators(nullptr)
  {}

  ArrayData(const ArrayData *src, ArrayKind kind,
            AllocationMode m = AllocationMode::smart)
    : m_kind(src->m_kind)
    , m_allocMode(m)
    , _count(0)
    , m_pos(src->m_pos)
    , m_strongIterators(nullptr)
  {}

  static HphpArray* Make(uint capacity);
  static HphpArray* Make(uint size, const TypedValue*);

  void destroy() {
    // If there are any strong iterators pointing to this array, they need
    // to be invalidated.
    if (UNLIKELY(m_strongIterators != nullptr)) freeStrongIterators();
  }

  ~ArrayData() { destroy(); }

  IMPLEMENT_COUNTABLE_METHODS

  /**
   * Create a new ArrayData with specified array element(s).
   */
  static ArrayData *Create();
  static ArrayData *Create(CVarRef value);
  static ArrayData *Create(CVarRef name, CVarRef value);
  static ArrayData *CreateRef(CVarRef value);
  static ArrayData *CreateRef(CVarRef name, CVarRef value);

  /**
   * Type conversion functions. All other types are handled inside Array class.
   */
  Object toObject() const;

  /**
   * Array interface functions.
   *
   * 1. For functions that return ArrayData pointers, these are the ones that
   *    can potentially escalate into a different ArrayData type. Return this
   *    if no escalation is needed.
   *
   * 2. All functions with a "key" parameter are type-specialized.
   */

  /**
   * For SmartAllocator.
   *
   *   NB: *Not* virtual. ArrayData knows about its only subclasses.
   */
  void release();

  /**
   * Whether this array has any element.
   */
  bool empty() const {
    return size() == 0;
  }

  /**
   * return the array kind for fast typechecks
   */
  ArrayKind kind() const {
    return m_kind;
  }

  /**
   * Number of elements this array has.
   */
  size_t size() const {
    if (UNLIKELY((int)m_size) < 0) return vsize();
    return m_size;
  }

  // unlike ArrayData::size(), this functions doesn't delegate
  // to the vsize() function, so its more efficient to use this when
  // you know you don't have a NameValueTableWrapper.
  size_t getSize() const {
    return m_size;
  }

  /**
   * Number of elements this array has.
   */
  size_t vsize() const;

  /**
   * getValueRef() gets a reference to value at position "pos".
   */
  CVarRef getValueRef(ssize_t pos) const;

  /*
   * Return true for array types that don't have COW semantics.
   */
  bool noCopyOnWrite() const;

  /*
   * Specific derived class type querying operators.
   */
  bool isPolicyArray() const { return m_kind == kPolicyKind; }
  bool isVector() const { return m_kind == kVectorKind; }
  bool isHphpArray() const {
    return m_kind <= kMixedKind;
    static_assert(kVectorKind < kMixedKind, "");
  }
  bool isSharedMap() const { return m_kind == kSharedKind; }
  bool isNameValueTableWrapper() const {
    return m_kind == kNvtwKind;
  }

  /*
   * Returns whether or not this array contains "vector-like" data.
   * I.e. iteration order produces int keys 0 to m_size-1 in sequence.
   */
  bool isVectorData() const;

  /**
   * Whether or not this array has a referenced Variant or Object appearing
   * twice. This is mainly for APC to decide whether to serialize an array.
   * Also used for detecting whether there is serializable object in the tree.
   */
  bool hasInternalReference(PointerSet &seen,
                            bool detectSerializable = false) const;

  /**
   * Position-based iterations, implemented using iter_begin,
   * iter_advance, iter_prev, iter_rewind.
   */
  Variant reset();
  Variant prev();
  Variant current() const;
  Variant next();
  Variant end();
  Variant key() const;
  Variant value(int32_t &pos) const;
  Variant each();

  bool isHead()            const { return m_pos == iter_begin(); }
  bool isTail()            const { return m_pos == iter_end(); }
  bool isInvalid()         const { return m_pos == invalid_index; }

  /**
   * Testing whether a key exists.
   */
  bool exists(int64_t k) const;
  bool exists(const StringData* k) const;

  /**
   * Interface for VM helpers.  ArrayData implements generic versions
   * using the other ArrayData api; subclasses may customize methods either
   * by providing a custom static method in g_array_funcs.
   */
  TypedValue* nvGet(int64_t k) const;
  TypedValue* nvGet(const StringData* k) const;
  void nvGetKey(TypedValue* out, ssize_t pos) const;

  // wrappers that call getValueRef()
  TypedValue* nvGetValueRef(ssize_t pos);
  Variant getValue(ssize_t pos) const;
  Variant getKey(ssize_t pos) const;

  /**
   * Getting l-value (that Variant pointer) at specified key. Return this if
   * escalation is not needed, or an escalated array data.
   */
  ArrayData *lval(int64_t k, Variant *&ret, bool copy);
  ArrayData *lval(StringData* k, Variant *&ret, bool copy);

  /**
   * Getting l-value (that Variant pointer) of a new element with the next
   * available integer key. Return this if escalation is not needed, or an
   * escalated array data. Note that adding a new element with the next
   * available integer key may fail, in which case ret is set to point to
   * the lval blackhole (see Variant::lvalBlackHole() for details).
   */
  ArrayData *lvalNew(Variant *&ret, bool copy);

  /**
   * Helper functions used for getting a reference to elements of
   * the dynamic property array in ObjectData or the local cache array
   * in ShardMap.
   */
  ArrayData *createLvalPtr(StringData* k, Variant *&ret, bool copy);
  ArrayData *getLvalPtr(StringData* k, Variant *&ret, bool copy);
  static ArrayData *CreateLvalPtr(ArrayData* ad, StringData* k,
                                  Variant *&ret, bool copy);
  static ArrayData *GetLvalPtr(ArrayData* ad, StringData* k,
                               Variant *&ret, bool copy);

  /**
   * Setting a value at specified key. If "copy" is true, make a copy first
   * then set the value. Return this if escalation is not needed, or an
   * escalated array data.
   */
  ArrayData *set(int64_t k, CVarRef v, bool copy);
  ArrayData *set(StringData* k, CVarRef v, bool copy);

  ArrayData *setRef(int64_t k, CVarRef v, bool copy);
  ArrayData *setRef(StringData* k, CVarRef v, bool copy);

  /**
   * The same as set(), but with the precondition that the key does
   * not already exist in this array.  (This is to allow more
   * efficient implementation of this case in some derived classes.)
   */
  ArrayData *add(int64_t k, CVarRef v, bool copy);
  ArrayData *add(StringData* k, CVarRef v, bool copy);

  /*
   * Same semantics as lval(), except with the precondition that the
   * key doesn't already exist in the array.
   */
  ArrayData *addLval(int64_t k, Variant *&ret, bool copy);
  ArrayData *addLval(StringData* k, Variant *&ret, bool copy);

  /**
   * Remove a value at specified key. If "copy" is true, make a copy first
   * then remove the value. Return this if escalation is not needed, or an
   * escalated array data.
   */
  ArrayData *remove(int64_t k, bool copy);
  ArrayData *remove(const StringData* k, bool copy);

  /**
   * Inline accessors that convert keys to StringData* before delegating to
   * the virtual method.  Helpers that take a CVarRef key dispatch to either
   * the StringData* or int64_t key-type helpers.
   */
  bool exists(CStrRef k) const;
  bool exists(CVarRef k) const;
  CVarRef get(int64_t k, bool error = false) const;
  CVarRef get(const StringData* k, bool error = false) const;
  CVarRef get(CStrRef k, bool error = false) const;
  CVarRef get(CVarRef k, bool error = false) const;
  ArrayData *lval(CStrRef k, Variant *&ret, bool copy);
  ArrayData *lval(CVarRef k, Variant *&ret, bool copy);
  ArrayData *createLvalPtr(CStrRef k, Variant *&ret, bool copy);
  ArrayData *getLvalPtr(CStrRef k, Variant *&ret, bool copy);
  ArrayData *set(CStrRef k, CVarRef v, bool copy);
  ArrayData *set(CVarRef k, CVarRef v, bool copy);
  ArrayData *set(const StringData*, CVarRef, bool) = delete;
  ArrayData *setRef(CStrRef k, CVarRef v, bool copy);
  ArrayData *setRef(CVarRef k, CVarRef v, bool copy);
  ArrayData *setRef(const StringData*, CVarRef, bool) = delete;
  ArrayData *add(CStrRef k, CVarRef v, bool copy);
  ArrayData *add(CVarRef k, CVarRef v, bool copy);
  ArrayData *addLval(CStrRef k, Variant *&ret, bool copy);
  ArrayData *addLval(CVarRef k, Variant *&ret, bool copy);
  ArrayData *remove(CStrRef k, bool copy);
  ArrayData *remove(CVarRef k, bool copy);

  ssize_t iter_begin() const;
  ssize_t iter_end() const;
  ssize_t iter_advance(ssize_t prev) const;
  ssize_t iter_rewind(ssize_t prev) const;

  /**
   * Mutable iteration APIs
   *
   * The following six methods are used for mutable iteration. For all methods
   * except newFullPos(), it is the caller's responsibility to ensure that the
   * specified FullPos 'fp' is registered with this array and hasn't already
   * been freed.
   */

  /**
   * Create a new mutable iterator and register it with this array (the mutable
   * iterator will be stored in 'fp'). The new iterator will point to whatever
   * element the array's internal cursor currently points to. Note that the
   * array keeps track of all mutable iterators that have registered with it.
   *
   * A mutable iterator remains live until one of the following happens:
   *   (1) The mutable iterator is freed by calling the freeFullPos() method.
   *   (2) The array's refcount drops to 0 and the array frees all mutable
   *       iterators that were registered with it.
   *   (3) Some other kind of "invalidation" event happens to the array that
   *       causes it to free all mutable iterators that were registered with
   *       it (ex. array_shift() is called on the array).
   */
  void newFullPos(FullPos &fp);

  /**
   * Frees a mutable iterator that was registered with this array.
   */
  void freeFullPos(FullPos &fp);

  /**
   * Checks if a mutable iterator points to a valid element within this array.
   * This will return false if the iterator points past the last element, or
   * if the iterator points before the first element.
   */
  bool validFullPos(const FullPos& fp) const;

  /**
   * Advances the mutable iterator to the next element in the array. Returns
   * false if the iterator has moved past the last element, otherwise returns
   * true.
   */
  bool advanceFullPos(FullPos& fp);

  CVarRef endRef();

  ArrayData* escalateForSort();
  void ksort(int sort_flags, bool ascending);
  void sort(int sort_flags, bool ascending);
  void asort(int sort_flags, bool ascending);
  void uksort(CVarRef cmp_function);
  void usort(CVarRef cmp_function);
  void uasort(CVarRef cmp_function);

  // default sort implementations
  static void Ksort(ArrayData*, int sort_flags, bool ascending);
  static void Sort(ArrayData*, int sort_flags, bool ascending);
  static void Asort(ArrayData*, int sort_flags, bool ascending);
  static void Uksort(ArrayData*, CVarRef cmp_function);
  static void Usort(ArrayData*, CVarRef cmp_function);
  static void Uasort(ArrayData*, CVarRef cmp_function);

  /**
   * Make a copy of myself.
   *
   * The nonSmartCopy() version means not to use the smart allocator.
   * Is only implemented for array types that need to be able to go
   * into the static array list.
   */
  ArrayData* copy() const;
  ArrayData* copyWithStrongIterators() const;
  ArrayData* nonSmartCopy() const;
  static ArrayData* CopyWithStrongIterators(const ArrayData*);
  static ArrayData* NonSmartCopy(const ArrayData*);

  /**
   * Append a value to the array. If "copy" is true, make a copy first
   * then append the value. Return NULL if escalation is not needed, or an
   * escalated array data.
   */
  ArrayData* append(CVarRef v, bool copy);
  ArrayData* appendRef(CVarRef v, bool copy);

  /**
   * Similar to append(v, copy), with reference in v preserved.
   */
  ArrayData* appendWithRef(CVarRef v, bool copy);

  /**
   * Implementing array appending and merging. If "copy" is true, make a copy
   * first then append/merge arrays. Return NULL if escalation is not needed,
   * or an escalated array data.
   */
  ArrayData* plus(const ArrayData *elems, bool copy);
  ArrayData* merge(const ArrayData *elems, bool copy);

  /**
   * Stack function: pop the last item and return it.
   */
  ArrayData* pop(Variant &value);

  /**
   * Queue function: remove the 1st item and return it.
   */
  ArrayData* dequeue(Variant &value);

  /**
   * Array function: prepend a new item.
   */
  ArrayData* prepend(CVarRef v, bool copy);

  /**
   * Only map classes need this. Re-index all numeric keys to start from 0.
   */
  void renumber();

  void onSetEvalScalar();

  /**
   * Serialize this array. We could have made this virtual function to ask
   * sub-classes to implement it specifically, but since this is not a critical
   * function to optimize, we implement it in a generic way in this base class.
   * Then all the sudden we find out all Zend HashTable functions are similar
   * to implementing array functions in this base class than utilizing a type
   * specialized implementation, which is normally more optimized.
   */
  void serialize(VariableSerializer *serializer,
                 bool skipNestCheck = false) const;

  void dump();
  void dump(std::string &out);
  void dump(std::ostream &os);

  /**
   * Comparisons.
   */
  int compare(const ArrayData *v2) const;
  bool equal(const ArrayData *v2, bool strict) const;

  void setPosition(ssize_t p) { m_pos = p; }

  ArrayData *escalate() const;

  // default implementations
  static ArrayData* Plus(ArrayData*, const ArrayData *elems, bool copy);
  static ArrayData* Merge(ArrayData*, const ArrayData *elems, bool copy);
  static ArrayData* Pop(ArrayData*, Variant &value);
  static ArrayData* Dequeue(ArrayData*, Variant &value);
  static ArrayData* Prepend(ArrayData*, CVarRef v, bool copy);
  static void Renumber(ArrayData*);
  static void OnSetEvalScalar(ArrayData*);
  static ArrayData* Escalate(const ArrayData* ad);

  static ArrayData *GetScalarArray(ArrayData *arr,
                                   const StringData *key = nullptr);
 private:
  void serializeImpl(VariableSerializer *serializer) const;
  static void compileTimeAssertions() {
    static_assert(offsetof(ArrayData, _count) == FAST_REFCOUNT_OFFSET,
                  "Offset of _count in ArrayData must be FAST_REFCOUNT_OFFSET");
  }

 protected:
  void freeStrongIterators();
  static void moveStrongIterators(ArrayData* dest, ArrayData* src);
  FullPos* strongIterators() const {
    return m_strongIterators;
  }
  void setStrongIterators(FullPos* p) {
    m_strongIterators = p;
  }
  // error-handling helpers
  static CVarRef getNotFound(int64_t k);
  static CVarRef getNotFound(const StringData* k);
  CVarRef getNotFound(int64_t k, bool error) const;
  CVarRef getNotFound(const StringData* k, bool error) const;
  static CVarRef getNotFound(CStrRef k);
  static CVarRef getNotFound(CVarRef k);

  static bool IsValidKey(CStrRef k);
  static bool IsValidKey(CVarRef k);
  static bool IsValidKey(const StringData* k) { return k; }

  // allocation helpers either call smart_malloc api or regular
  // malloc, depending on m_allocMode
  void* modeAlloc(size_t nbytes) const;
  void* modeRealloc(void* ptr, size_t nbytes) const;
  void modeFree(void* ptr) const;

 protected:
  ArrayKind m_kind;
  const AllocationMode m_allocMode;
  uint32_t m_size;
  mutable int32_t _count;
  int32_t m_pos;
 private:
  FullPos* m_strongIterators; // head of linked list

 public: // for the JIT
  static uint32_t getKindOff() {
    return (uintptr_t)&((ArrayData*)0)->m_kind;
  }

 public: // for heap profiler
  void getChildren(std::vector<TypedValue *> &out);
};

/*
 * ArrayFunctions is a hand-built virtual dispatch table.  Each field represents
 * one virtual method with an array of function pointers, one per ArrayKind.
 * There is one global instance of this table.  Arranging it this way allows
 * dispatch to be done with a single indexed load, using m_kind as the index.
 */
struct ArrayFunctions {
  // NK stands for number of array kinds; here just for shorthand.
  static auto const NK = size_t(ArrayData::ArrayKind::kNumKinds);
  void (*release[NK])(ArrayData*);
  TypedValue* (*nvGetInt[NK])(const ArrayData*, int64_t k);
  TypedValue* (*nvGetStr[NK])(const ArrayData*, const StringData* k);
  void (*nvGetKey[NK])(const ArrayData*, TypedValue* out, ssize_t pos);
  ArrayData* (*setInt[NK])(ArrayData*, int64_t k, CVarRef v, bool copy);
  ArrayData* (*setStr[NK])(ArrayData*, StringData* k, CVarRef v, bool copy);
  size_t (*vsize[NK])(const ArrayData*);
  CVarRef (*getValueRef[NK])(const ArrayData*, ssize_t pos);
  bool noCopyOnWrite[NK];
  bool (*isVectorData[NK])(const ArrayData*);
  bool (*existsInt[NK])(const ArrayData*, int64_t k);
  bool (*existsStr[NK])(const ArrayData*, const StringData* k);
  ArrayData* (*lvalInt[NK])(ArrayData*, int64_t k, Variant*& ret,
                            bool copy);
  ArrayData* (*lvalStr[NK])(ArrayData*, StringData* k, Variant*& ret,
                            bool copy);
  ArrayData* (*lvalNew[NK])(ArrayData*, Variant *&ret, bool copy);
  ArrayData* (*createLvalPtr[NK])(ArrayData*, StringData* k, Variant *&ret,
                                  bool copy);
  ArrayData* (*getLvalPtr[NK])(ArrayData*, StringData* k, Variant *&ret,
                               bool copy);
  ArrayData* (*setRefInt[NK])(ArrayData*, int64_t k, CVarRef v, bool copy);
  ArrayData* (*setRefStr[NK])(ArrayData*, StringData* k, CVarRef v, bool copy);
  ArrayData* (*addInt[NK])(ArrayData*, int64_t k, CVarRef v, bool copy);
  ArrayData* (*addStr[NK])(ArrayData*, StringData* k, CVarRef v, bool copy);
  ArrayData* (*addLvalInt[NK])(ArrayData*, int64_t k, Variant *&ret,
                               bool copy);
  ArrayData* (*addLvalStr[NK])(ArrayData*, StringData* k, Variant *&ret,
                               bool copy);
  ArrayData* (*removeInt[NK])(ArrayData*, int64_t k, bool copy);
  ArrayData* (*removeStr[NK])(ArrayData*, const StringData* k, bool copy);
  ssize_t (*iterBegin[NK])(const ArrayData*);
  ssize_t (*iterEnd[NK])(const ArrayData*);
  ssize_t (*iterAdvance[NK])(const ArrayData*, ssize_t pos);
  ssize_t (*iterRewind[NK])(const ArrayData*, ssize_t pos);
  bool (*validFullPos[NK])(const ArrayData*, const FullPos&);
  bool (*advanceFullPos[NK])(ArrayData*, FullPos&);
  ArrayData* (*escalateForSort[NK])(ArrayData*);
  void (*ksort[NK])(ArrayData* ad, int sort_flags, bool ascending);
  void (*sort[NK])(ArrayData* ad, int sort_flags, bool ascending);
  void (*asort[NK])(ArrayData* ad, int sort_flags, bool ascending);
  void (*uksort[NK])(ArrayData* ad, CVarRef cmp_function);
  void (*usort[NK])(ArrayData* ad, CVarRef cmp_function);
  void (*uasort[NK])(ArrayData* ad, CVarRef cmp_function);
  ArrayData* (*copy[NK])(const ArrayData*);
  ArrayData* (*copyWithStrongIterators[NK])(const ArrayData*);
  ArrayData* (*nonSmartCopy[NK])(const ArrayData*);
  ArrayData* (*append[NK])(ArrayData*, CVarRef v, bool copy);
  ArrayData* (*appendRef[NK])(ArrayData*, CVarRef v, bool copy);
  ArrayData* (*appendWithRef[NK])(ArrayData*, CVarRef v, bool copy);
  ArrayData* (*plus[NK])(ArrayData*, const ArrayData* elems, bool copy);
  ArrayData* (*merge[NK])(ArrayData*, const ArrayData* elems, bool copy);
  ArrayData* (*pop[NK])(ArrayData*, Variant& value);
  ArrayData* (*dequeue[NK])(ArrayData*, Variant& value);
  ArrayData* (*prepend[NK])(ArrayData*, CVarRef value, bool copy);
  void (*renumber[NK])(ArrayData*);
  void (*onSetEvalScalar[NK])(ArrayData*);
  ArrayData* (*escalate[NK])(const ArrayData*);
};

extern const ArrayFunctions g_array_funcs;

ALWAYS_INLINE inline
void decRefArr(ArrayData* arr) {
  if (arr->decRefCount() == 0) arr->release();
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_ARRAY_DATA_H_

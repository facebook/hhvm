/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_RESOURCE_DATA_H_
#define incl_HPHP_RESOURCE_DATA_H_

#ifndef incl_HPHP_INSIDE_HPHP_COMPLEX_TYPES_H_
#error Directly including 'resource_data.h' is prohibited. \
       Include 'complex_types.h' instead.
#endif

#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/sweepable.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Base class of all PHP resources.
 */
class ResourceData {
 private:
  static DECLARE_THREAD_LOCAL_NO_CHECK(int, os_max_resource_id);

 public:
  ResourceData();

 private:
  // Disallow copy construction
  ResourceData(const ResourceData&) = delete;

 public:
  void setStatic() const { assert(false); }
  bool isStatic() const { return false; }
  IMPLEMENT_COUNTABLENF_METHODS_NO_STATIC

  virtual ~ResourceData(); // all PHP resources need vtables

  void operator delete(void* p) { ::operator delete(p); }

  void release() {
    assert(getCount() == 0);
    delete this;
  }

  Class* getVMClass() const {
    return m_cls;
  }

  static size_t getVMClassOffset() {
    // For assembly linkage.
    size_t res = offsetof(ResourceData, m_cls);
    assert(res == ObjectData::getVMClassOffset());
    return res;
  }

  int32_t o_getId() const { return o_id; }
  void o_setId(int id); // only for BuiltinFiles
  static int GetMaxResourceId() ATTRIBUTE_COLD;

  CStrRef o_getClassName() const;
  virtual CStrRef o_getClassNameHook() const;
  virtual CStrRef o_getResourceName() const;
  virtual bool isInvalid() const { return false; }

  bool o_toBoolean() const { return 1; }
  int64_t o_toInt64() const { return o_id; }
  double o_toDouble() const { return o_id; }
  String o_toString() const {
    return String("Resource id #") + String(o_id);
  }
  Array o_toArray() const;

  void serialize(VariableSerializer* serializer) const;
  void serializeImpl(VariableSerializer* serializer) const;
  void dump() const;

 private:
  static void compileTimeAssertions() {
    static_assert(offsetof(ResourceData, m_count) == FAST_REFCOUNT_OFFSET, "");
  }

  //============================================================================
  // ResourceData fields

 protected:
  // Numeric identifier of resource object (used by var_dump() and other
  // output functions)
  int32_t o_id;
  // Counter to keep track of the number of references to this resource
  // (i.e. the resource's "refcount")
  mutable RefCount m_count;
  // Pointer to the __resource class; this field is needed (and must be at
  // the same offset as ObjectData::m_cls) so that backup gc and other things
  // that walk the SmartAllocator heaps can distinguish between objects and
  // resources
  Class* m_cls;
  // Storage for dynamic properties
  ArrNR o_properties;

} __attribute__((aligned(16)));

/**
 * Rules to avoid memory problems/leaks from ResourceData classes
 * ==============================================================
 *
 * 1. If a ResourceData is entirely smart allocated, for example,
 *
 *    class EntirelySmartAllocated : public ResourceData {
 *    public:
 *       int number; // primitives are allocated together with "this"
 *       String str; // smart-allocated objects are fine
 *    };
 *
 *    Then, the best choice is to use these two macros to make sure the object
 *    is always collected during request shutdown time:
 *
 *       DECLARE_OBJECT_ALLOCATION(T);
 *       IMPLEMENT_OBJECT_ALLOCATION(T);
 *
 *    This object doesn't participate in sweep(), as object allocator doesn't
 *    have any callback installed.
 *
 * 2. If a ResourceData is entirely not smart allocated, for example,
 *
 *    class NonSmartAllocated : public SweepableResourceData {
 *    public:
 *       int number; // primitives are always not in consideration
 *       std::string str; // this has malloc() in its own
 *       std::vector<int> vec; // all STL collection classes belong here
 *       HANDLE ptr; // raw pointers that need to be free-d somehow
 *    };
 *
 *    Then it has to derive from SweepableResourceData, so sweep() will be
 *    called. By default, it will call this object's destructor automatically,
 *    so everything will be free-d.
 *
 *    When deriving from SweepableResourceData, either "new" or "NEW" can
 *    be used, but we prefer people use NEW with these macros:
 *
 *       DECLARE_OBJECT_ALLOCATION(T);
 *       IMPLEMENT_OBJECT_ALLOCATION(T);
 *
 * 3. If a ResourceData is a mix of smart allocated data members and non-
 *    smart allocated data members, sweep() has to be overwritten to only
 *    free non-smart allocated data members. This is because smart allocated
 *    data members may have their own sweep() defined to destruct, and another
 *    destruction from this ResourceData's default sweep() will cause double-
 *    free problems on these smart allocated data members.
 *
 *    This means, std::vector<String> is almost always wrong, because there is
 *    no way to free up vector's memory without touching String, which is
 *    smart allocated.
 *
 *    class MixedSmartAllocated : public SweepableResourceData {
 *    public:
 *       int number; // primitives are always not in consideration
 *
 *       // STL classes need to new/delete to have clean sweep
 *       std::string *stdstr;
 *       std::vector<int> *vec;
 *
 *       HANDLE ptr; // raw pointers that need to be free-d somehow
 *       String str; // smart-allocated objects are fine
 *
 *       DECLARE_OBJECT_ALLOCATION(T);
 *    };
 *    IMPLEMENT_OBJECT_ALLOCATION_NO_DEFAULT_SWEEP(T);
 *    void MixedSmartAllocated::sweep() {
 *       delete stdstr;
 *       delete vec;
 *       close_handle(ptr);
 *       // without doing anything with Strings, Arrays, or Objects
 *    }
 *
 * 4. If a ResourceData may be persistent, it cannot use object allocation. It
 *    then has to derive from SweepableResourceData, because a new-ed pointer
 *    can only be collected/deleted by sweep().
 *
 */
class SweepableResourceData : public ResourceData, public Sweepable {};

typedef std::map<std::string, ResourceData*> ResourceMap;
typedef std::map<std::string, ResourceMap> ResourceMapMap;

///////////////////////////////////////////////////////////////////////////////

// Suppress the default implementation of the SmartPtr destructor so that
// derived classes (ex. HPHP::Resource) can manually handle decReffing the
// ResourceData.
template<> inline SmartPtr<ResourceData>::~SmartPtr() {}

ALWAYS_INLINE void decRefRes(ResourceData* res) {
  if (res->decRefCount() == 0) res->release();
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_RESOURCE_DATA_H_

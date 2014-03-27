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

#ifndef incl_HPHP_RESOURCE_DATA_H_
#define incl_HPHP_RESOURCE_DATA_H_

#include "hphp/runtime/base/countable.h"
#include "hphp/runtime/base/sweepable.h"

#include "hphp/util/thread-local.h"

namespace HPHP {

class Array;
class String;
class VariableSerializer;

///////////////////////////////////////////////////////////////////////////////

/**
 * Base class of all PHP resources.
 */
class ResourceData {
 private:
  static __thread int os_max_resource_id;

 public:
  static void resetMaxId() { os_max_resource_id = 0; }

  ResourceData();

 private:
  // Disallow copy construction
  ResourceData(const ResourceData&) = delete;

 public:
  void setStatic() const { assert(false); }
  bool isStatic() const { return false; }
  void setUncounted() const { assert(false); }
  bool isUncounted() const { return false; }
  IMPLEMENT_COUNTABLENF_METHODS_NO_STATIC

  virtual ~ResourceData(); // all PHP resources need vtables

  void operator delete(void* p) { ::operator delete(p); }

  void release() {
    assert(!hasMultipleRefs());
    delete this;
  }

  int32_t o_getId() const { return o_id; }
  void o_setId(int id); // only for BuiltinFiles

  const String& o_getClassName() const;
  virtual const String& o_getClassNameHook() const;
  virtual const String& o_getResourceName() const;
  virtual bool isInvalid() const { return false; }

  bool o_toBoolean() const { return true; }
  int64_t o_toInt64() const { return o_id; }
  double o_toDouble() const { return o_id; }
  String o_toString() const;
  Array o_toArray() const;

  void serialize(VariableSerializer* serializer) const;
  void serializeImpl(VariableSerializer* serializer) const;

 private:
  static void compileTimeAssertions();

  //============================================================================
  // ResourceData fields

 protected:
  // Numeric identifier of resource object (used by var_dump() and other
  // output functions)
  int32_t o_id;
  // Counter to keep track of the number of references to this resource
  // (i.e. the resource's "refcount")
  mutable RefCount m_count;
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
class SweepableResourceData : public ResourceData, public Sweepable {
protected:
  void sweep() FOLLY_OVERRIDE {
    // ResourceData objects are non-smart allocated by default (see
    // operator delete in ResourceData), so sweeping will destroy the
    // object and deallocate its seat as well.
    delete this;
  }
};

///////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE bool decRefRes(ResourceData* res) {
  return res->decRefAndRelease();
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_RESOURCE_DATA_H_

/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "hphp/runtime/base/complex_types.h"
#include "hphp/runtime/base/memory/sweepable.h"
#include "hphp/runtime/vm/instance.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Base class of all resources used by extensions for opaquely passing object
 * pointers.
 */
class ResourceData : public Instance {
public:
  ResourceData();
  virtual ~ResourceData();
  void operator delete(void* p) { ::operator delete(p); }

  // implementing ObjectData
  virtual bool isResource() const { return true;}
  virtual String t___tostring();
  virtual int64_t o_toInt64() const { return o_getId();}
  void o_setId(int id); // only for BuiltinFiles
  virtual void serializeImpl(VariableSerializer *serializer) const;
  virtual CStrRef o_getResourceName() const;
  virtual int o_getResourceId() const { return o_getId(); }

  static int GetMaxResourceId() ATTRIBUTE_COLD;

  static const bool IsResourceClass = true;
};

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
}

#endif // incl_HPHP_RESOURCE_DATA_H_

/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __HPHP_FIBER_REFERENCE_MAP_H__
#define __HPHP_FIBER_REFERENCE_MAP_H__

#include <util/base.h>
#include <runtime/base/fiber_async_func.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Referenced pointer (strongly bound variants and objects) mapping between
 * mother thread and fiber.
 */
class FiberReferenceMap {
public:
  void insert(ObjectData *src, ObjectData *copy);
  void insert(Variant *src, Variant *copy);

  void *lookup(void *src);
  void *reverseLookup(void *copy);

  bool empty() const { return m_forward_references.empty();}

  // gets called by generated fiber_marshal_global_state()
  template<typename T>
  void marshal(T &dest, T &src) {
    dest = src;
  }
  void marshal(String  &dest, String  &src);
  void marshal(Array   &dest, Array   &src);
  void marshal(Object  &dest, Object  &src);
  void marshal(Variant &dest, Variant &src);

  // gets called by generated fiber_unmarshal_global_state()
  template<typename T>
  void unmarshal(T &dest, T &src, char strategy) {
    if (strategy != FiberAsyncFunc::GlobalStateIgnore) {
      dest = src;
    }
  }
  void unmarshal(String  &dest, String  &src, char strategy);
  void unmarshal(Array   &dest, Array   &src, char strategy);
  void unmarshal(Object  &dest, Object  &src, char strategy);
  void unmarshal(Variant &dest, CVarRef  src, char strategy);

  void unmarshalDynamicGlobals
  (Array &dest, Array &src, char default_strategy,
   const hphp_string_map<char> &additional_strategies);

private:
  PointerMap m_forward_references;
  PointerMap m_reverse_references;
  Array m_refVariants;

  void insert(void *src, void *copy);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_FIBER_REFERENCE_MAP_H__

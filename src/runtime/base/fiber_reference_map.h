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
  FiberReferenceMap();
  void reset();

  void insert(ObjectData *src, ObjectData *copy);
  void insert(Variant *src, Variant *copy, bool marshaling);

  void *lookup(void *src);
  void *reverseLookup(void *copy);

  bool empty() const { return m_forward_references.empty();}

  void marshal(bool    &dest,    bool src) { dest = src;}
  void marshal(int64   &dest,   int64 src) { dest = src;}
  void marshal(double  &dest,  double src) { dest = src;}
  void marshal(String  &dest, CStrRef src);
  void marshal(Array   &dest, CArrRef src);
  void marshal(Object  &dest, CObjRef src);
  void marshal(Variant &dest, CVarRef src);

  void unmarshal(bool    &dest, bool     src, char strategy) {
    if (strategy != FiberAsyncFunc::GlobalStateIgnore) dest = src;
  }
  void unmarshal(int64   &dest, int64    src, char strategy) {
    if (strategy != FiberAsyncFunc::GlobalStateIgnore) dest = src;
  }
  void unmarshal(double  &dest, double   src, char strategy) {
    if (strategy != FiberAsyncFunc::GlobalStateIgnore) dest = src;
  }
  void unmarshal(String  &dest, CStrRef  src, char strategy);
  void unmarshal(Array   &dest, CArrRef  src, char strategy);
  void unmarshal(Object  &dest, CObjRef  src, char strategy);
  void unmarshal(Variant &dest, CVarRef  src, char strategy);

  void unmarshalDynamicGlobals
  (Array &dest, CArrRef src, char default_strategy,
   const hphp_string_map<char> &additional_strategies);

private:
  PointerMap m_forward_references;
  PointerMap m_reverse_references;

  Array *m_mainRefVariants;
  Array m_fiberRefVariants;

  void insert(void *src, void *copy);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_FIBER_REFERENCE_MAP_H__

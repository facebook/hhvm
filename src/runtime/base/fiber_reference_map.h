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

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Referenced pointer (strongly bound variants and objects) mapping between
 * mother thread and fiber.
 */
class FiberReferenceMap {
public:
  void insert(void *src, void *copy);
  void *lookup(void *src);
  void *reverseLookup(void *copy);

  bool empty() const { return m_forward_references.empty();}

private:
  PointerMap m_forward_references;
  PointerMap m_reverse_references;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_FIBER_REFERENCE_MAP_H__

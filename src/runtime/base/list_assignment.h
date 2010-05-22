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

#ifndef __HPHP_LIST_ASSIGNMENT_H__
#define __HPHP_LIST_ASSIGNMENT_H__

#include <runtime/base/types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Helper class for transforming ListAssignment expression.
 */
class ListAssignmentElement {
 public:
  /**
   * "var" is the l-value Variant to take some element of an array on right-
   * hand side. "index" and ... are nested array indices flatten into a vector
   * by code generator. For example,
   *
   *   list($a, list($b), $c) = $data = array(1, array(2), 3);
   *
   * is transformed into,
   *
   *   list_assign($data, LAE($a, 0), LAE($b, 1, 0), LAE($c, 2), NULL);
   */
  ListAssignmentElement(Variant &var, int index, ...);

  /**
   * Called by list_assign to assign one value. "data" is the array element
   * to assign to the Variant & this object holds.
   */
  void assign(CArrRef data);

 private:
  Variant &m_var;             // l-value variant to assign to
  std::vector<int> m_indices; // flattened nested array indices
};

/**
 * This could be moved to builtin_functions, but list assignment is really
 * special and localized.
 */
Variant list_assign(CVarRef data, ListAssignmentElement *elem, ...);

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_LIST_ASSIGNMENT_H__

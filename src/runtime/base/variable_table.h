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

#ifndef __HPHP_VARIABLE_TABLE_H__
#define __HPHP_VARIABLE_TABLE_H__

#include <runtime/base/types.h>
#include <runtime/base/complex_types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * R-value variable table that can look up a variable's value by its name. Code
 * generator will generate a subclass like this,
 *
 *   class VariableTable : public RVariableTable {
 *     public:
 *       virtual Variant getImpl(const char *s) {
 *         if (strcmp(s, "a") == 0) return v_a;
 *         // ...
 *       }
 *   };
 */
class RVariableTable : public Array {
 public:
  virtual ~RVariableTable() {}
  Variant get(CVarRef s) { return getImpl(s.toString());}
  Variant get(CStrRef s) { return getImpl(s);}
  Variant get(litstr  s) { return getImpl(s);}

  /**
   * Code-generated sub-class may override this function by generating one
   * entry per variable.
   */
  virtual bool exists(CStrRef s) const {
    // Integers are never valid variable names.
    return Array::exists(s, true);
  }

  /**
   * Code-generated sub-class will implement this function by generating one
   * entry per variable.
   */
  virtual Variant getImpl(CStrRef s) = 0;

  virtual Array getDefinedVars() const;
};

///////////////////////////////////////////////////////////////////////////////

/**
 * L-value variable table that can get/set a variable's value by its name. The
 * reason we have both RVariableTable and LVariableTable, instead of just this
 * one, is because LVariableTable requires all variables to be Variant type,
 * taking away type inference. If we can tell no dynamic variable was used in
 * l-value context, we don't have to use LVariableTable. Of course, ideally
 * even RVariableTable is not needed, meaning no dynamic variable is ever used.
 */
class LVariableTable : public Array {
 public:
  virtual ~LVariableTable() {}
  Variant &get(CVarRef s) { return getImpl(s.toString()); }
  Variant &get(CStrRef s) { return getImpl(s); }
  Variant &get(litstr  s) { return getImpl(s);}

  /**
   * Code-generated sub-class may override this function by generating one
   * entry per variable.
   */
  virtual bool exists(CStrRef s) const {
    return Array::exists(s, true);
  }

  /**
   * Code-generated sub-class will implement this function by generating one
   * entry per variable.
   */
  virtual Variant &getImpl(CStrRef s);

  virtual Array getDefinedVars();
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_VARIABLE_TABLE_H__

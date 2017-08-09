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

#ifndef incl_HPHP_PHP7_LVALUE_H
#define incl_HPHP_PHP7_LVALUE_H

#include "hphp/runtime/vm/hhbc.h"
#include "hphp/php7/zend/zend.h"
#include "hphp/php7/bytecode.h"
#include "hphp/php7/cfg.h"

namespace HPHP { namespace php7 {

/* This is a value that we can take a base pointer to--different from Lvalue
 * since PHP has values that can be indexed but not assigned to: namely, array
 * expressions
 */
struct BaseValue {
  virtual ~BaseValue() = default;

  struct MinstrSeq;
  virtual CFG getB(MinstrSeq& seq) = 0;
};

/* This is a PHP value that acts like an lvalue--that is, it can be both read
 * from or assigned to. Mostly this corresponds to locals, but other lvalues
 * are object properties or elements of an array
 */
struct Lvalue : BaseValue {
  /* Get an lvalue for an expression--if the expression is not an lvalue,
   * returns nullptr; */
  static std::unique_ptr<Lvalue> getLvalue(const zend_ast* ast);

  virtual CFG getC() = 0;
  virtual CFG getV() = 0;
  virtual CFG getF(uint32_t slot) = 0;
  virtual CFG assign(const zend_ast* rhs) = 0;
  virtual CFG bind(const zend_ast* rhs) = 0;
  virtual CFG assignOp(SetOpOp op, const zend_ast* rhs) = 0;
  virtual CFG incDec(IncDecOp op) = 0;
};

}} // HPHP::php7

#endif // incl_HPHP_PHP7_LVALUE_H

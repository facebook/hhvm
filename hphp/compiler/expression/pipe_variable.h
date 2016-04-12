/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_PIPE_VARIABLE_H_
#define incl_HPHP_PIPE_VARIABLE_H_

#include "hphp/compiler/expression/expression.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(PipeVariable);

struct PipeVariable : Expression {
  PipeVariable(EXPRESSION_CONSTRUCTOR_PARAMETERS);

  DECLARE_BASE_EXPRESSION_VIRTUAL_FUNCTIONS;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_PIPE_VARIABLE_H_

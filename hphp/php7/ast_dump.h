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

#ifndef incl_HPHP_PHP7_AST_DUMP_H_
#define incl_HPHP_PHP7_AST_DUMP_H_

#include <hphp/php7/zend/zend.h>

#include <ostream>

namespace HPHP { namespace PHP7 {

void dump_ast(
    std::ostream& out,
    zend_ast* ast,
    bool pretty = true,
    unsigned int indent = 0);

}} // HPHP::PHP7

#endif //incl_HPHP_PHP7_AST_DUMP_H_

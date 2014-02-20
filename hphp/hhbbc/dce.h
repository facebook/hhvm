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
#ifndef incl_HHBBC_DCE_H_
#define incl_HHBBC_DCE_H_

#include <vector>

#include "hphp/hhbbc/misc.h"

namespace HPHP { namespace HHBBC {

struct Index;
struct State;
struct Context;
struct Bytecode;
namespace php { struct Block; }

//////////////////////////////////////////////////////////////////////

std::vector<Bytecode> local_dce(const Index&,
                                Context,
                                borrowed_ptr<php::Block>,
                                const State&);

//////////////////////////////////////////////////////////////////////

}}

#endif

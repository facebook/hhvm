/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#ifndef incl_HHBBC_COMPRESSION_H_
#define incl_HHBBC_COMPRESSION_H_

#include "hphp/hhbbc/bc.h"
#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/representation.h"

namespace HPHP { namespace HHBBC { namespace compression {

//////////////////////////////////////////////////////////////////////

using Buffer = std::vector<char>;

void decodeBytecodeVec(const Buffer& buffer, BytecodeVec& bcs);
void encodeBytecodeVec(Buffer& buffer, const BytecodeVec& bcs);

// Run a correctness test of compression: compress and decompress all
// php::Func in the given program. Although we take a mutable ref to the
// program as input, this operation shouldn't change the program's code.
void testCompression(php::Program& program);

//////////////////////////////////////////////////////////////////////

}}}

#endif

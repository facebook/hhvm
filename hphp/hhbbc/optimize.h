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
#ifndef incl_HPHP_OPTIMIZE_H_
#define incl_HPHP_OPTIMIZE_H_

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

struct Index;
struct FuncAnalysis;

/*
 * Use information from an analyze call to perform various
 * optimizations on a function.
 *
 * The Index should be unchanged since the one that was provided to
 * the corresponding analyze_func call.
 *
 * This routine may modify the php::Blocks attached to the passed-in
 * php::Func, but it won't modify the top-level meta-data in the
 * php::Func itself.
 */
void optimize_func(const Index&, FuncAnalysis);

//////////////////////////////////////////////////////////////////////

}}

#endif

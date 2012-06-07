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

#ifndef incl_TRANSL_TYPES_H_
#define incl_TRANSL_TYPES_H_

#include <hash_map>
#include <hash_set>

namespace HPHP {
namespace VM {
namespace Transl {

/*
 * Core types.
 */
typedef unsigned char* TCA; // "Translation cache adddress."
typedef const unsigned char* CTCA;

using std::vector;
// XXX: the g++ hash primitives are clowny.
using __gnu_cxx::hash_map;
using __gnu_cxx::hash_set;

}}}

#endif

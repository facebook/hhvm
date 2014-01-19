/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_VM_PROFILER_H_
#define incl_HPHP_VM_PROFILER_H_

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/runtime.h"

namespace HPHP {

/*
 * Create a data structure to hold type profile information.  This
 * structure is logically a map with the structure:
 *
 *   (function, parameter index, type) -> observed count
 *
 * In this map "type" is a tuple containing both the PHP DataType and,
 * if the type is KindOfObject, the Class of the object.
 */
void initTypeProfileStructure();

/*
 * Record an observation of (func, paramIndex, type of value)
 */
void profileOneArgument(TypedValue value, int32_t paramIndex, const Func* func);

/*
 * Record observation of all types passed to a function.
 */
void profileAllArguments(ActRec* ar);

/*
 * Return an array containing function's type profile, where counts
 * are expressed as a percentage of total observations.
 */
Array getPercentParamInfoArray(const Func* func);

/*
 * Format the type profile as JSON and write it to disk at
 * /tmp/type-profile.txt
 */
void writeProfileInformationToDisk();

}

#endif

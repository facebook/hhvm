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

#pragma once

#include <folly/DiscriminatedPtr.h>

namespace HPHP {

struct ArrayData;
struct Class;
struct Func;
struct NamedEntity;
struct StringData;
struct Unit;

namespace data_map {
/*
 * Logical reverse mapping from memory address ranges to the VM objects
 * allocated at them.
 */

///////////////////////////////////////////////////////////////////////////////

/*
 * Register the start address of an object.
 *
 * The object pointer must be 16-byte aligned.
 */
void register_start(const ArrayData*);
void register_start(const Class*);
void register_start(const Func*);
void register_start(const NamedEntity*);
void register_start(const StringData*);
void register_start(const Unit*);

/*
 * Deregister an object.
 *
 * Requires that register_start() was called on the object, and it has not yet
 * been deregistered.
 */
void deregister(const ArrayData*);
void deregister(const Class*);
void deregister(const Func*);
void deregister(const NamedEntity*);
void deregister(const StringData*);
void deregister(const Unit*);

/*
 * Look up the object containing `addr' as an internal pointer.
 *
 * This function is aware of the variable-length allocations of Func, Class,
 * and StringData, and takes them into consideration for determining
 * constituency.
 *
 * Returns an empty result if `addr' is not mapped.
 */
using result = folly::DiscriminatedPtr<
  ArrayData,
  Class,
  Func,
  NamedEntity,
  StringData,
  Unit
>;
result find_containing(const void* addr);

///////////////////////////////////////////////////////////////////////////////

}}


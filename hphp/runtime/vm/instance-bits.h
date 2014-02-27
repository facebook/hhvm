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
#ifndef incl_HPHP_RUNTIME_VM_INSTANCE_BITS_H_
#define incl_HPHP_RUNTIME_VM_INSTANCE_BITS_H_

#include <bitset>
#include <atomic>
#include <cinttypes>

#include "hphp/util/mutex.h"

namespace HPHP { struct StringData; }

//////////////////////////////////////////////////////////////////////

/*
 * During warmup, we profile the most common classes or interfaces
 * involved in instanceof checks in order to set up a bitmask for each
 * class to allow these checks to be performed quickly by the JIT.
 */
namespace HPHP { namespace InstanceBits {

//////////////////////////////////////////////////////////////////////

typedef std::bitset<128> BitSet;

/*
 * The initFlag tracks whether init() has finished yet.
 *
 * The lock is used to protect access to the instance bits on
 * individual classes to ensure that after the initFlag -> true
 * transition, all loaded classes will properly have instance bits
 * populated.
 *
 * See Unit::defClass for some details.
 */
extern ReadWriteMutex lock;
extern std::atomic<bool> initFlag;

/*
 * Called to record an instanceof check for `name', during the warmup
 * phase.
 */
void profile(const StringData* name);

/*
 * InstanceBits::init() must be called by the first translation which
 * uses instance bits, while holding the write lease.
 */
void init();

/*
 * Returns: the instance bit for the class or interface `name', or
 * zero if there is no allocated bit.
 *
 * This function may be called by the thread doing init(), or
 * otherwise only after init() is finished (i.e. initFlag == true).
 */
unsigned lookup(const StringData* name);

/*
 * Populate a mask and offset for checking instance bits from JIT
 * compiled code.  The offset is the offset of the byte that should be
 * tested with mask, relative to a Class*.
 *
 * Returns false if `name' has no instance bit.
 *
 * Pre: initFlag == true.
 */
bool getMask(const StringData* name, int& offset, uint8_t& mask);

//////////////////////////////////////////////////////////////////////

}}

#endif

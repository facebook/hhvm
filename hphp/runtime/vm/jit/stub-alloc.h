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

#ifndef incl_HPHP_JIT_STUB_ALLOC_H_
#define incl_HPHP_JIT_STUB_ALLOC_H_

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/util/data-block.h"

#include <set>

namespace HPHP { namespace jit {

struct CGMeta;

/*
 * Allocate or free an epehemeral service request stub.
 *
 * getRequestStub() returns the address of a freed stub if one is available;
 * otherwise, it returns frozen.frontier().  If not nullptr, `isReused' is
 * set to whether or not the returned stub is being reused.
 *
 * Note that we don't track the sizes of stubs anywhere---this code only
 * works because all service requests emit a code segment of size
 * svcreq::stub_size().
 *
 * Pre: tc::assertOwnsMetadataLock
 */
TCA allocTCStub(CodeBlock& frozen, CGMeta* fixups,
                     bool* isReused = nullptr);
void markStubFreed(TCA stub);

/*
 * Return a set of all unused request stubs currently on the free list.
 */
std::set<TCA> getFreeTCStubs();

}}

#endif

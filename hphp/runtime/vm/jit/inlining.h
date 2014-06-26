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

#ifndef incl_HPHP_JIT_INLINING_H_
#define incl_HPHP_JIT_INLINING_H_

namespace HPHP { namespace JIT {
///////////////////////////////////////////////////////////////////////////////

struct NormalizedInstruction;
struct RegionDesc;

///////////////////////////////////////////////////////////////////////////////

/*
 * Can we perform inlining at `inst' from within `region'?
 *
 * This is a shallow check---it asks whether `inst' is an FCall{,D} with an
 * appropriate FPush* in the same region, and verifies that the call does not
 * block inlining (e.g., due to missing arguments, recursion, resumable callee,
 * etc.).
 *
 * It does not peek into the callee's bytecode or regions, and it is
 * insensitive to inlining depth and other inlining context.
 *
 * NOTE: Inlining will fail during translation if the FPush was interpreted.
 * It is up to the client to ensure that this is not the case.
 */
bool canInlineAt(const NormalizedInstruction& inst,
                 const RegionDesc& region);

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_JIT_INLINING_H_

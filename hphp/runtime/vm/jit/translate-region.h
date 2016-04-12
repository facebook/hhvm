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
#ifndef incl_HPHP_TRANSLATE_REGION_H_
#define incl_HPHP_TRANSLATE_REGION_H_

#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/prof-src-key.h"

namespace HPHP { namespace jit {

struct IRGS;
struct IRUnit;
struct TransContext;

//////////////////////////////////////////////////////////////////////

/*
 * Populate and optimize an IRUnit for `region'. Returns the new IRUnit on
 * success, nullptr on failure.
 *
 * `pconds' and `annotations' must be empty on entry, and if hhir generation is
 * successful they will be populated appropriately.
 */
std::unique_ptr<IRUnit> irGenRegion(const RegionDesc& region,
                                    const TransContext& context,
                                    PostConditions& pconds,
                                    Annotations& annotations) noexcept;

//////////////////////////////////////////////////////////////////////

}}

#endif

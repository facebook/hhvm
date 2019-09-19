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

#ifndef incl_HPHP_PROPERTY_PROFILE_H_
#define incl_HPHP_PROPERTY_PROFILE_H_

#include <cinttypes>

#include "hphp/runtime/base/string-data.h"

namespace HPHP {

struct StringData;
namespace jit {
struct ProfDataSerializer;
struct ProfDataDeserializer;
}

namespace PropertyProfile {

/*
 * Increment the profile count for cls::prop.
 */
void incCount(const StringData* cls, const StringData* prop);

/*
 * Get the profile count for cls::prop.
 */
uint32_t getCount(const StringData* cls, const StringData* prop);

/*
 * Serialize and deserialize.
 */
void serialize(jit::ProfDataSerializer&);
void deserialize(jit::ProfDataDeserializer&);

} }

#endif

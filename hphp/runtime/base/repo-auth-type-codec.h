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
#ifndef incl_HPHP_REPO_AUTH_TYPE_CODEC_H_
#define incl_HPHP_REPO_AUTH_TYPE_CODEC_H_

#include "hphp/runtime/base/repo-auth-type.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct UnitEmitter;
struct Unit;

//////////////////////////////////////////////////////////////////////

/*
 * Decode a RepoAuthType in the format expected in opcode streams.
 *
 * If the RepoAuthType carries strings, they are stored as Id's, and
 * how to look up the Id depends on whether we're pulling it out of a
 * UnitEmitter or a Unit, so it's overloaded.
 *
 * The `pc' parameter is expected to be a pointer into a serialized
 * bytecode array, and is advanced by the amount we consumed.
 */
RepoAuthType decodeRAT(const Unit*, const unsigned char*& pc);
RepoAuthType decodeRAT(const UnitEmitter&, const unsigned char*& pc);

/*
 * Return the size of an encoded RepoAuthType.
 *
 * This is for parsing bytecode (when you don't need to actually
 * create the RepoAuthType).
 */
size_t encodedRATSize(const unsigned char* pc);

/*
 * Encode a RepoAuthType into a UnitEmitter's bytecode stream, in the
 * format used by decodeRAT.
 *
 * This function also merges any litstrs into the unit as appropriate.
 */
void encodeRAT(UnitEmitter& ue, RepoAuthType rat);

//////////////////////////////////////////////////////////////////////

}

#include "hphp/runtime/base/repo-auth-type-codec-inl.h"

#endif

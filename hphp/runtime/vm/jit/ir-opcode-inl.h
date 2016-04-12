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

namespace HPHP {
namespace jit {

struct OpInfo {
  const char* name;
  uint64_t flags;
};
extern OpInfo g_opInfo[];

inline const char* opcodeName(Opcode opcode) {
  return g_opInfo[uint16_t(opcode)].name;
}

inline bool opcodeHasFlags(Opcode opcode, uint64_t flags) {
  return g_opInfo[uint16_t(opcode)].flags & flags;
}

inline bool hasEdges(Opcode opcode) {
  return opcodeHasFlags(opcode, Branch | MayRaiseError);
}

inline bool opHasExtraData(Opcode op) {
  return opcodeHasFlags(op, HasExtra);
}

} // namespace jit
} // namespace HPHP

/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | (c) Copyright IBM Corporation 2015-2016                              |
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

#include "hphp/ppc64-asm/dasm-ppc64.h"

#include <folly/Format.h>

#include "hphp/ppc64-asm/asm-ppc64.h"
#include "hphp/ppc64-asm/decoder-ppc64.h"

namespace ppc64_asm {

void Disassembler::disassembly(std::ostream& out,
                               const uint8_t* const instr,
                               const uint8_t* const address) {
  if (!color_.empty()) {
    out << color_;
  }

  for (int i=0; i < indent_level_; i++) {
     out << ' ';
  }

  int pos;
  uint32_t instruction = 0;

  for (pos = 0; pos < instr_size_in_bytes; ++pos) {
     // TODO(rcardoso): only print encoding if told so.
     out << folly::format("{:02x} ", (uint8_t)instr[pos]);
     instruction |= ((uint8_t)instr[pos]) << (8 * pos);
  }
  out << "\t";
  // Decode instruction and get mnemonic representation
  DecoderInfo dec_info = Decoder::GetDecoder().decode(instr);
  if (address) dec_info.setIp(address);
  out << dec_info.toString();
  out << "\n";
}

} // namespace ppc64_asm

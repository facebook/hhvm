/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <iomanip>
#include <stdlib.h>

#include "folly/Format.h"

#include "util/disasm.h"

#include "util/base.h"

namespace HPHP {

Disasm::Disasm(int indentLevel /* = 0 */, bool printEncoding /* = false */)
    : m_indent(indentLevel)
    , m_printEncoding(printEncoding)
{
#ifdef HAVE_LIBXED
  xed_state_init(&m_xedState, XED_MACHINE_MODE_LONG_64,
                 XED_ADDRESS_WIDTH_64b, XED_ADDRESS_WIDTH_64b);
  xed_tables_init();
#endif // HAVE_LIBXED
}

#ifdef HAVE_LIBXED
static void error(std::string msg) {
  fprintf(stderr, "Error: %s\n", msg.c_str());
  exit(1);
}

#define MAX_INSTR_ASM_LEN 128

static const xed_syntax_enum_t s_xed_syntax =
  getenv("HHVM_ATT_DISAS") ? XED_SYNTAX_ATT : XED_SYNTAX_INTEL;
#endif // HAVE_LIBXED

void Disasm::disasm(std::ostream& out, uint8_t* codeStartAddr,
                    uint8_t* codeEndAddr) {
#ifdef HAVE_LIBXED
  char codeStr[MAX_INSTR_ASM_LEN];
  xed_uint8_t *frontier;
  xed_decoded_inst_t xedd;
  uint64 ip;

  // Decode and print each instruction
  for (frontier = codeStartAddr, ip = (uint64)codeStartAddr;
       frontier < codeEndAddr; ) {
    xed_decoded_inst_zero_set_mode(&xedd, &m_xedState);
    xed_decoded_inst_set_input_chip(&xedd, XED_CHIP_INVALID);
    xed_error_enum_t xed_error = xed_decode(&xedd, frontier, 15);
    if (xed_error != XED_ERROR_NONE) error("disasm error: xed_decode failed");

    // Get disassembled instruction in codeStr
    if (!xed_format_context(s_xed_syntax, &xedd, codeStr,
                            MAX_INSTR_ASM_LEN, ip, nullptr)) {
      error("disasm error: xed_format_context failed");
    }

    for (int i = 0; i < m_indent; ++i) {
      out << ' ';
    }
    out << folly::format("{:#10x}: ", ip);

    uint32 instrLen = xed_decoded_inst_get_length(&xedd);
    if (m_printEncoding) {
      // print encoding, like in objdump
      unsigned posi = 0;
      for (; posi < instrLen; ++posi) {
        out << folly::format("{:02x} ", (uint8_t)frontier[posi]);
      }
      for (; posi < 16; ++posi) {
        out << "   ";
      }
    }
    out << codeStr << std::endl;
    frontier += instrLen;
    ip       += instrLen;
  }
#else
  out << "This binary was compiled without disassembly support\n";
#endif // HAVE_LIBXED
}

} // namespace HPHP

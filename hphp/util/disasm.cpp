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
#include "hphp/util/disasm.h"

#include <iomanip>
#include <stdlib.h>
#include <execinfo.h>
#include <cxxabi.h>

#include "folly/Format.h"
#include "folly/ScopeGuard.h"

#include "hphp/util/base.h"
#include "hphp/util/text-color.h"

namespace HPHP {

#ifdef HAVE_LIBXED
// Parse the function name out of backtrace_symbols output, which
// looks like:
//     filename(function+offset) [addr]
//
// It's possible that we just have "[addr]", if there's no symbol
// associated, so return NULL in that case.
//
// Following the (bad?) example of backtrace_symbols and
// __cxa_demangle, this function allocates memory with malloc if
// successful, and doesn't allocate otherwise.
static char *getFunctionName(char *backtraceName) {
  char *fnStart = strchr(backtraceName, '(');
  char *fnEnd = strrchr(backtraceName, '+');
  if (!fnStart || !fnEnd || fnStart > fnEnd) {
    return NULL;
  }
  fnStart++;
  int length = fnEnd - fnStart;
  char *functionName = (char *)malloc(length + 1);
  strncpy(functionName, fnStart, length);
  functionName[length] = '\0';
  return functionName;
}

// Parse the method name out of a demangled name, which looks like:
//     namespace::class::method(type::args...)
// Allocates with malloc when successful, and doesn't if it isn't.
static char *getMethodName(char *demangledName) {
  char *p = demangledName + strlen(demangledName);
  char *lparen = NULL;
  char *colon = NULL;
  while (*p != '(') {
    p--;
    if (p < demangledName) {
      return NULL;
    }
  }
  lparen = p;
  while (*p != ':') {
    p--;
    if (p < demangledName) {
      return NULL;
    }
  }
  colon = p;
  int length = lparen - (colon + 1);
  char *methodName = (char *)malloc(length + 1);
  strncpy(methodName, colon + 1, length);
  methodName[length] = '\0';
  return methodName;
}

static hphp_hash_map<xed_uint64_t,const char*> addressToSymbolMemo;

// XED callback function to get a symbol from an address
static int addressToSymbol(xed_uint64_t address,
                           char *symbol_buffer,
                           xed_uint32_t buffer_length,
                           xed_uint64_t *offset,
                           void *context) {
  auto& memoVal = addressToSymbolMemo[address];
  if (memoVal != nullptr) {
    strncpy(symbol_buffer, memoVal, buffer_length - 1);
    symbol_buffer[buffer_length - 1] = '\0';
    *offset = 0;
    return 1;
  }

  char **symbolTable = backtrace_symbols((void **)&address, 1);
  if (!symbolTable) {
    return 0;
  }
  char *backtraceName = symbolTable[0];
  char *mangledName = getFunctionName(backtraceName);
  free(symbolTable);
  if (!mangledName) {
    return 0;
  }

  int status;
  char *demangledName = abi::__cxa_demangle(mangledName, NULL, NULL, &status);
  char *methodName = NULL;
  if (status == 0) {
    free(mangledName);
    methodName = getMethodName(demangledName);
    free(demangledName);
  } else if (status == -2) {
    methodName = mangledName;
  } else {
    free(mangledName);
  }

  if (methodName) {
    strncpy(symbol_buffer, methodName, buffer_length - 1);
    symbol_buffer[buffer_length - 1] = '\0';
    memoVal = methodName;
  } else {
    return 0;
  }

  *offset = 0;
  return 1;
}
#endif /* HAVE_LIBXED */

Disasm::Disasm(const Disasm::Options& opts)
    : m_opts(opts)
{
#ifdef HAVE_LIBXED
  xed_state_init(&m_xedState, XED_MACHINE_MODE_LONG_64,
                 XED_ADDRESS_WIDTH_64b, XED_ADDRESS_WIDTH_64b);
  xed_tables_init();
  xed_register_disassembly_callback(addressToSymbol);
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
  auto const endClr = m_opts.m_color.empty() ? "" : ANSI_COLOR_END;
  char codeStr[MAX_INSTR_ASM_LEN];
  xed_uint8_t *frontier;
  xed_decoded_inst_t xedd;
  uint64_t codeBase = uint64_t(codeStartAddr);
  uint64_t ip;

  // Decode and print each instruction
  for (frontier = codeStartAddr, ip = (uint64_t)codeStartAddr;
       frontier < codeEndAddr; ) {
    xed_decoded_inst_zero_set_mode(&xedd, &m_xedState);
    xed_decoded_inst_set_input_chip(&xedd, XED_CHIP_INVALID);
    xed_error_enum_t xed_error = xed_decode(&xedd, frontier, 15);
    if (xed_error != XED_ERROR_NONE) error("disasm error: xed_decode failed");

    // Get disassembled instruction in codeStr
    auto const syntax = m_opts.m_forceAttSyntax ? XED_SYNTAX_ATT
                                                : s_xed_syntax;
    if (!xed_format_context(syntax, &xedd, codeStr,
                            MAX_INSTR_ASM_LEN, ip, nullptr)) {
      error("disasm error: xed_format_context failed");
    }
    uint32_t instrLen = xed_decoded_inst_get_length(&xedd);

    // If it's a jump, we're printing relative offsets, and the dest
    // is within the range we're printing, add the dest as a relative
    // offset.
    std::string jmpComment;
    auto const cat = xed_decoded_inst_get_category(&xedd);
    if (cat == XED_CATEGORY_COND_BR || cat == XED_CATEGORY_UNCOND_BR) {
      if (m_opts.m_relativeOffset) {
        auto disp = uint64_t(frontier + instrLen +
                             xed_decoded_inst_get_branch_displacement(&xedd) -
                             codeBase);
        if (disp < uint64_t(codeEndAddr - codeStartAddr)) {
          jmpComment = folly::format(" # {:#x}", disp).str();
        }
      }
    }

    for (int i = 0; i < m_opts.m_indentLevel; ++i) {
      out << ' ';
    }
    out << m_opts.m_color;
    if (m_opts.m_addresses) {
      const char* fmt = m_opts.m_relativeOffset ? "{:3x}: " : "{:#10x}: ";
      out << folly::format(fmt, ip - (m_opts.m_relativeOffset ? codeBase : 0));
    }
    if (m_opts.m_printEncoding) {
      // print encoding, like in objdump
      unsigned posi = 0;
      for (; posi < instrLen; ++posi) {
        out << folly::format("{:02x} ", (uint8_t)frontier[posi]);
      }
      for (; posi < 16; ++posi) {
        out << "   ";
      }
    }
    out << codeStr << jmpComment << endClr << '\n';
    frontier += instrLen;
    ip       += instrLen;
  }
#else
  out << "This binary was compiled without disassembly support\n";
#endif // HAVE_LIBXED
}

} // namespace HPHP

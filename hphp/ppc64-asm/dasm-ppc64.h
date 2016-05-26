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

#ifndef INCLUDE_DASM_PPC64_H_
#define INCLUDE_DASM_PPC64_H_

#include <ostream>

namespace ppc64_asm {

struct Disassembler {
 public:
  Disassembler()
   : print_encoding_(false)
   , print_address_(false)
   , indent_level_(0)
   , color_(nullptr)
   {}

  Disassembler(bool print_enc,
     bool print_addr, int indent_level, std::string color)
   : print_encoding_(print_enc)
   , print_address_(print_addr)
   , indent_level_(indent_level)
   , color_(color)
   {}

  ~Disassembler() {}

  void disassembly(std::ostream& out, uint8_t* instr);

 private:
  bool print_encoding_;
  bool print_address_;
  int indent_level_;
  std::string color_;
};

} // namespace ppc64_asm

#endif

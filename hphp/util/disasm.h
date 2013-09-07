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
#ifndef incl_HPHP_UTIL_DISASM_H_
#define incl_HPHP_UTIL_DISASM_H_

#ifdef HAVE_LIBXED
extern "C" {
#include <xed-interface.h>
}
#endif // HAVE_LIBXED

#include <ostream>

#include <boost/noncopyable.hpp>

namespace HPHP {

class Disasm : private boost::noncopyable {
 public:
  struct Options {
    Options()
      : m_indentLevel(0)
      , m_printEncoding(false)
      , m_relativeOffset(false)
      , m_addresses(true)
      , m_forceAttSyntax(false)
    {}

    Options& indent(int i) {
      m_indentLevel = i;
      return *this;
    }

    Options& printEncoding(bool pe) {
      m_printEncoding = pe;
      return *this;
    }

    Options& relativeOffset(bool re) {
      m_relativeOffset = re;
      return *this;
    }

    Options& color(std::string c) {
      m_color = std::move(c);
      return *this;
    }

    Options& addresses(bool b) {
      m_addresses = b;
      return *this;
    }

    Options& forceAttSyntax(bool b) {
      m_forceAttSyntax = b;
      return *this;
    }

    int m_indentLevel;
    bool m_printEncoding;
    bool m_relativeOffset;
    bool m_addresses;
    bool m_forceAttSyntax;
    std::string m_color;
  };

  /* Create a Disasm object. indentLevel spaces will be put at the beginning of
   * each line of disassembly. If printEncoding is true, the raw hex bytes of
   * the instructions will also be in the output. */
  explicit Disasm(const Options& opts = Options());

  /* Disassemble instructions. start should be the first byte of the region to
   * disassemble and end should be the first byte past the region to
   * disassemble. */
  void disasm(std::ostream& out, uint8_t* start, uint8_t* end);

 private:
#ifdef HAVE_LIBXED
  xed_state_t m_xedState;
#endif // HAVE_LIBXED
  const Options m_opts;
};

} // namespace HPHP

#endif

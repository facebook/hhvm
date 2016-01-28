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

#ifndef incl_HPHP_VM_BC_PATTERN_H_
#define incl_HPHP_VM_BC_PATTERN_H_

#include "hphp/runtime/vm/hhbc.h"

#include <initializer_list>
#include <unordered_set>
#include <utility>
#include <vector>

namespace HPHP {

class Func;

//////////////////////////////////////////////////////////////////////

/**
 * A minimally-featured pattern-matcher for HHBC with marginal utility and
 * questionable design.
 *
 * This class was made for a very specific purpose (matching singleton
 * patterns)---if we want something better, we should probably find some nice
 * codegen library that does it for us.  (For instance, we have some support
 * for "alternation", except that if we ever succeed in matching a branch,
 * we'll never revisit it).
 *
 * To use this class, instantiate a BCPattern with a list of opcodes and
 * metacharacters, then matchAnchored() on a PC.
 */
class BCPattern {
public:
  ////////////////////////////////////////////////////////////////////

  class Atom;

  typedef std::vector<Atom> Expr;
  typedef std::vector<PC> CaptureVec;

  /**
   * Opcode plus pattern-matching metadata.
   *
   * For JmpZ and JmpNZ, also supports pattern matching on the taken branch.
   */
  class Atom {
  public:
    /* implicit */ Atom(Op op)
      : m_op(op)
      , m_capture(false)
      , m_filter(nullptr)
    {}

    Op op() const { return m_op; }

    typedef std::function<bool(PC, const CaptureVec&)> FilterFunc;

    /**
     * Metadata getters.
     */
    bool shouldCapture() const { return m_capture; }
    FilterFunc getFilter() const { return m_filter; }
    const Expr& getTaken() const { return m_taken; }

    bool hasAlt() const { return m_alt.size() != 0; }
    bool hasSeq() const { return m_seq.size() != 0; }
    const Expr& getAlt() const { return m_alt; }
    const Expr& getSeq() const { return m_seq; }

    /**
     * Metadata setters.
     */
    Atom& capture() {
      m_capture = true;
      return *this;
    }

    Atom& onlyif(FilterFunc filter) {
      m_filter = filter;
      return *this;
    }

    Atom& taken(std::initializer_list<Atom> pattern) {
      assert(m_op == Op::JmpZ || m_op == Op::JmpNZ);

      m_taken = pattern;
      return *this;
    }

    template <typename... Atoms>
    static Atom alt(Atom first, Atoms&&... rest) {
      auto res = first;
      res.m_alt = Expr { rest... };
      return res;
    }

    template <typename... Atoms>
    static Atom seq(Atom first, Atoms&&... rest) {
      auto res = first;
      res.m_seq = Expr { rest... };
      return res;
    }

  private:
    Op m_op;
    bool m_capture;
    FilterFunc m_filter;
    Expr m_alt;
    Expr m_seq;
    Expr m_taken;
  };

  ////////////////////////////////////////////////////////////////////

  /**
   * Result of a pattern match.
   */
  class Result {
  public:
    bool found() const  { return m_start; }

    PC getStart() const { return m_start; }
    PC getEnd() const   { return m_end; }

    PC getCapture(int i) const {
      if (i >= m_captures.size()) {
        return nullptr;
      }
      return m_captures[i];
    }

  private:
    void erase() {
      m_start = 0;
      m_end = 0;
      m_captures.clear();
    }

    PC m_start{0};
    PC m_end{0};
    CaptureVec m_captures;

    friend class BCPattern;
  };

  ////////////////////////////////////////////////////////////////////

public:
  BCPattern(std::initializer_list<Atom> pattern) : m_pattern(pattern) {}

  /**
   * Global "flags" for the pattern.
   */
  BCPattern& ignore(std::initializer_list<Op> atoms) {
    m_ignores.insert(atoms);
    return *this;
  }

  /**
   * Match against bytecode.
   */
  Result matchAnchored(const Func* func);
  Result matchAnchored(PC start, PC end);

private:
  PC next(PC pc) {
    return pc + instrLen(pc);
  }

  void matchAnchored(const Expr& pattern, PC start, PC end, Result& result);

private:
  Expr m_pattern;
  std::unordered_set<Op> m_ignores;
};

//////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_VM_BC_PATTERN_H_

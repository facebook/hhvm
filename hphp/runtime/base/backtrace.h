/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_BACKTRACE_H_
#define incl_HPHP_BACKTRACE_H_

namespace HPHP {

class Array;
class VMParserFrame;

struct BacktraceArgs {

  /**
   * creates the backtrace using the settings from BacktraceArgs
   * example: createBacktrace(BacktraceArgs().skipTop().setLimit(numOfFrames));
   */
  friend Array createBacktrace(const BacktraceArgs& backtraceArgs);

  /**
   * Skip the top frame of the stack. Used to jump over internal frames.
   */
  BacktraceArgs& skipTop(bool skipTop = true) {
    m_skipTop = skipTop;
    return *this;
  }

  /**
   * Include the current frame on top of the stack. If both skipTop and withSelf
   * are set then first frame is skipped and the second is added.
   */
  BacktraceArgs& withSelf(bool withSelf = true) {
    m_withSelf = withSelf;
    return *this;
  }

  /**
   * Return the object associated with each frame.
   */
  BacktraceArgs& withThis(bool withThis = true) {
    m_withThis = withThis;
    return *this;
  }

  /**
   * Return the metadata associated with each frame having 86metadata local.
   */
  BacktraceArgs& withMetadata(bool withMetadata = true) {
    m_withMetadata = withMetadata;
    return *this;
  }

  /**
   * Do not return function arguments for frames on the stack.
   */
  BacktraceArgs& ignoreArgs(bool ignoreArgs = true) {
    m_withArgNames = m_withArgNames && !ignoreArgs;
    m_withArgValues = m_withArgValues && !ignoreArgs;
    return *this;
  }

  /**
   * Print out the first x frames up to some limit x.
   */
  BacktraceArgs& setLimit(int limit) {
    m_limit = limit;
    return *this;
  }

  /**
   * The parser frame when a file has a compile time error and a backtrace
   * is needed.
   */
  BacktraceArgs& setParserFrame(VMParserFrame* parserFrame) {
    m_parserFrame = parserFrame;
    return *this;
  }

  /**
   * Include the pseudo main in the backtrace.
   */
  BacktraceArgs& withPseudoMain(bool withPseudoMain = true) {
    m_withPseudoMain = withPseudoMain;
    return *this;
  }

  /**
   * Include argument values in backtrace.
   */
  BacktraceArgs& withArgValues(bool withValues = true) {
    m_withArgValues = withValues;
    return *this;
  }

  /**
   * Include argument names in backtrace.
   */
  BacktraceArgs& withArgNames(bool withNames = true) {
    m_withArgNames = withNames;
    return *this;
  }

private:
  bool m_skipTop{false};
  bool m_withSelf{false};
  bool m_withThis{false};
  bool m_withMetadata{false};
  bool m_withPseudoMain{false};
  bool m_withArgValues{true};
  bool m_withArgNames{false};
  int m_limit{0};
  VMParserFrame* m_parserFrame{nullptr};
};

Array createBacktrace(const BacktraceArgs& backtraceArgs);

} // HPHP

#endif // incl_HPHP_BACKTRACE_H_

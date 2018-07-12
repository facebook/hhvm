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

#ifndef incl_HPHP_PROCESS_EXEC_H_
#define incl_HPHP_PROCESS_EXEC_H_

#include <string>

namespace HPHP { namespace proc {
////////////////////////////////////////////////////////////////////////////////

/*
 * Execute an external program.  Unlike folly::Subprocess, this works with MSVC.
 *
 * @param path   binary file's full path
 * @param argv   argument array
 * @param in     stdin
 * @param out    stdout
 * @param err    stderr; NULL for don't care
 * @return       true if program was executed, even if there was stderr;
 *               false if anything failed and unable to run the specified
 *               program
 */
bool exec(const char* path, const char* argv[], const char* in,
          std::string& out, std::string* err = nullptr, bool color = false);

/*
 * Daemonize current process.
 */
void daemonize(const char* stdoutFile = "/dev/null",
               const char* stderrFile = "/dev/null");

/*
 * RAII guard to temporarily re-enable forking.
 */
struct EnableForkInDebuggerGuard {
  EnableForkInDebuggerGuard();
  ~EnableForkInDebuggerGuard();

  static bool isForkEnabledInDebugger();
};

////////////////////////////////////////////////////////////////////////////////
}}

#endif

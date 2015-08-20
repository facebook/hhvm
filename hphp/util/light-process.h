/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_LIGHT_PROCESS_H_
#define incl_HPHP_LIGHT_PROCESS_H_

#ifdef _MSC_VER
# error LightProcess is not supported under MSVC!
#endif

#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <map>

#include <signal.h>
#include <unistd.h>

#include "hphp/util/process.h"
#include "hphp/util/lock.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// light-weight process

class LightProcess {
public:
  LightProcess();
  ~LightProcess();

  static void Close();
  static bool Available();
  static void Initialize(const std::string &prefix, int count,
                         bool trackProcessTimes,
                         const std::vector<int> &inherited_fds);
  static void ChangeUser(const std::string &username);

  typedef std::function<void(pid_t)> LostChildHandler;
  static void SetLostChildHandler(const LostChildHandler& handler);

  static FILE *popen(const char *cmd, const char *type,
                     const char *cwd = nullptr);
  static int pclose(FILE *f);

  /**
   * Opens a process with given cwd and environment variables.
   *
   * The parameters "created" and "desired" describe the pipes that need to
   * be setup for the child process: "created" contains created fd for child,
   * and "desired" contains desired fd in child.
   *
   * The parameter env contains strings of the form <name>=<content>.
   */
  static pid_t proc_open(const char *cmd, const std::vector<int> &created,
                         const std::vector<int> &desired,
                         const char *cwd, const std::vector<std::string> &env);

  /**
   * The main process is not the (direct) parent of the worker process,
   * and therefore it has to delegate to the shadow process to do waitpid.
   * There can be a timeout (in seconds), after which SIGKILL is sent to
   * the child process.
   */
  static pid_t waitpid(pid_t pid, int *stat_loc, int options, int timeout = 0);

  static pid_t pcntl_waitpid(pid_t pid, int *stat_loc, int options);

private:
  static int GetId();
  static void SigChldHandler(int sig, siginfo_t* info, void* ctx);

  bool initShadow(int afdt_listen,
                  const std::string& afdt_filename, int id,
                  const std::vector<int> &inherited_fds);
  static void runShadow(int afdt_fd);
  void closeShadow();

  /**
   * For later light processes to close their pipes to previous ones.
   */
  void closeFiles();

  static FILE *LightPopenImpl(const char *cmd, const char *type,
                              const char *cwd);
  static FILE *HeavyPopenImpl(const char *cmd, const char *type,
                              const char *cwd);

  pid_t m_shadowProcess;
  Mutex m_procMutex;
  int m_afdt_fd;
  std::map<FILE*, pid_t> m_popenMap;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_LIGHT_PROCESS_H_

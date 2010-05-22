/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __LIGHT_PROCESS_H__
#define __LIGHT_PROCESS_H__

#include "process.h"
#include "lock.h"
#include <string>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// light-weight process

class LightProcess {
public:
  LightProcess();
  ~LightProcess();

  static void Close();
  static bool Available();
  static void Initialize(const std::string &prefix, int count);
  static void ChangeUser(const std::string &username);

  static FILE *popen(const char *cmd, const char *type);
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
   */
  static pid_t waitpid(pid_t pid, int *stat_loc, int options);

  static pid_t pcntl_waitpid(pid_t pid, int *stat_loc, int options);

private:
  static int GetId();

  bool initShadow(const std::string &prefix, int id);
  void runShadow(int fdin, int fdout);
  void closeShadow();

  pid_t m_shadowProcess;
  FILE *m_fin;   // the pipe to read from the child
  FILE *m_fout;  // the pipe to write to the child
  Mutex m_procMutex;
  std::string m_afdtFilename;
  int m_afdt_fd;
  std::map<int64, int64> m_popenMap;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __LIGHT_PROCESS_H__

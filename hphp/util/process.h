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

#ifndef incl_HPHP_PROCESS_H_
#define incl_HPHP_PROCESS_H_

#include <string>
#include <vector>
#include <cstdio>

#include <sys/types.h>
#ifdef _MSC_VER
# include <windows.h>
#else
# include <sys/syscall.h>
# include <unistd.h>
#endif
#include <pthread.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// helper class

class CPipe {
public:
  CPipe()  { m_fds[0] = m_fds[1] = 0;}
  ~CPipe() { close();}

  bool open()  { close(); return !pipe(m_fds);}
  void close() {
    for (int i = 0; i <= 1; i++) {
      if (m_fds[i]) {
        ::close(m_fds[i]);
        m_fds[i] = 0;
      }
    }
  }

  int getIn() const  { return m_fds[1];}
  int getOut() const { return m_fds[0];}
  int detachIn()  { int fd = m_fds[1]; m_fds[1] = 0; return fd;}
  int detachOut() { int fd = m_fds[0]; m_fds[0] = 0; return fd;}
  bool dupIn2(int fd) { return dup2(m_fds[1], fd) >= 0;}
  bool dupOut2(int fd) { return dup2(m_fds[0], fd) >= 0;}

private:
  int m_fds[2];
};

///////////////////////////////////////////////////////////////////////////////

class Process {
public:
  // Cached process statics
  static std::string HostName;
  static std::string CurrentWorkingDirectory;
  static void InitProcessStatics();

  /**
   * Current executable's name.
   */
  static std::string GetAppName();

  /**
   * This machine'a name.
   */
  static std::string GetHostName();

  /**
   * Process identifier.
   */
  static pid_t GetProcessId() {
    return getpid();
  }

  /**
   * Parent's process identifier.
   */
  static pid_t GetParentProcessId() {
    return getppid();
  }

  /**
   * Search for a process by command line. If matchAll is false, only binary
   * file's name, not the whole path + command line options, will be matched.
   */
  static pid_t GetProcessId(const std::string &cmd, bool matchAll = false);
  static void GetProcessId(const std::string &cmd, std::vector<pid_t> &pids,
                           bool matchAll = false);

  /**
   * Get command line with a process ID.
   */
  static std::string GetCommandLine(pid_t pid);

  /**
   * Check if the current process is being run under GDB.  Will return false if
   * we're unable to read /proc/{Process::GetProcessId()}/status.
   */
  static bool IsUnderGDB();

  /**
   * Get memory usage in MB by a process.
   */
  static int64_t GetProcessRSS(pid_t pid);

  /**
   * Current thread's identifier.
   */
  static pthread_t GetThreadId() {
    return pthread_self();
  }

  /**
   * Current thread's identifier.
   */
  static uint64_t GetThreadIdForTrace() {
    // For tracing purposes this just needs to be unique, pthread_t is not
    // portable but even if it's a pointer to a struct like on OSX this will
    // produce a unique value. If we support platforms where this isn't the
    // case we will need to revisit this.
#ifdef __linux__
    return pthread_self();
#else
    return (uint64_t)pthread_self();
#endif
  }

  /*
   * Thread's process identifier.
   */
  static pid_t GetThreadPid() {
#ifdef __FreeBSD__
# if __FreeBSD__version > 900030
    return pthread_getthreadid_np();
# else
    long tid;
    syscall(SYS_thr_self, &tid);
    return (pid_t) tid;
# endif
#elif defined(__CYGWIN__) || defined(__MINGW__)
    return (long)pthread_self();
#elif defined(_MSC_VER)
    return GetCurrentThreadId();
#else
    return syscall(SYS_gettid);
#endif
  }

  /**
   * Are we in the main thread still?
   */
  static bool IsInMainThread() {
    return Process::GetThreadPid() == Process::GetProcessId();
  }

  /**
   * Get CPU information.
   */
  static int GetCPUCount();
  static std::string GetCPUModel();

  /**
   * Get binary code footprint in bytes.
   */
  static size_t GetCodeFootprint(pid_t pid);

  /**
   * Get current working directory.
   */
  static std::string GetCurrentDirectory();

  /**
   * Get current user's name.
   */
  static std::string GetCurrentUser();

  /**
   * Get current user's home directory.
   */
  static std::string GetHomeDirectory();

public:
  /**
   * Execute an external program.
   *
   * @param   path   binary file's full path
   * @param   argv   argument array
   * @param   in     stdin
   * @param   out    stdout
   * @param   err    stderr; NULL for don't care
   * @return         true if program was executed, even if there was stderr;
   *                 false if anything failed and unable to run the specified
   *                 program
   */
  static bool Exec(const char *path, const char *argv[], const char *in,
                   std::string &out, std::string *err = nullptr,
                   bool color = false);

  /**
   * Execute an external program.
   *
   * @param   cmd    command line
   * @param   outf   save stdout to this file
   * @param   errf   save stderr to this file
   * @return         exit code of the program
   */
  static int Exec(const std::string &cmd, const std::string &outf,
                  const std::string &errf);

  /**
   * Daemonize current process.
   */
  static void Daemonize(const char *stdoutFile = "/dev/null",
                        const char *stderrFile = "/dev/null");

private:
  static int Exec(const char *path, const char *argv[], int *fdin, int *fdout,
                  int *fderr
#ifdef _MSC_VER
                  , PROCESS_INFORMATION* procInfo
#endif
                 );
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_PROCESS_H_

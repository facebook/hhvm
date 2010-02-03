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

#include "process.h"
#include <boost/shared_ptr.hpp>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <vector>
#include "base.h"
#include "util.h"
#include "async_func.h"

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// helpers

static void swap_fd(const string &filename, FILE *fdesc) {
  FILE *f = fopen(filename.c_str(), "a");
  if (f == NULL || dup2(fileno(f), fileno(fdesc)) < 0) {
    if (f) fclose(f);
    _exit(-1);
  }
}

///////////////////////////////////////////////////////////////////////////////

class FileReader {
public:
  FileReader(FilePtr f, string &out) : m_f(f), m_out(out) {}
  void read() { readString(m_f.get(), m_out); }

  static void readString(FILE *f, string &out) {
    size_t nread = 0;
    const unsigned int BUFFER_SIZE = 1024;
    char buf[BUFFER_SIZE];
    while ((nread = fread(buf, 1, BUFFER_SIZE, f)) != 0) {
      out.append(buf, nread);
    }
  }

private:
  FilePtr m_f;
  string &m_out;
};

///////////////////////////////////////////////////////////////////////////////

static void sigChildHandler(int signum) {
  wait(0);
}

// Cached process statics
std::string Process::HostName;
std::string Process::CurrentWorkingDirectory;

void Process::InitProcessStatics() {
  HostName = GetHostName();
  CurrentWorkingDirectory = GetCurrentDirectory();
}

bool Process::Exec(const char *path, const char *argv[], const char *in,
                   string &out, string *err /* = NULL */) {
  signal(SIGCHLD, sigChildHandler);

  int fdin = 0; int fdout = 0; int fderr = 0;
  int pid = Exec(path, argv, &fdin, &fdout, &fderr);
  if (pid == 0) return false;

  FilePtr sout(fdopen(fdout, "r"), file_closer());
  FilePtr serr(fdopen(fderr, "r"), file_closer());
  {
    FilePtr sin(fdopen(fdin, "w"), file_closer());
    if (!sin) return false;
    if (in && *in) {
      fwrite(in, 1, strlen(in), sin.get());
    }
  }
  if (!sout || !serr) return false;

  FileReader outReader(sout, out);
  AsyncFunc<FileReader> func(&outReader, &FileReader::read);
  func.start();

  string junk;
  if (err == NULL) err = &junk; // hzhao: don't know if this is needed
  FileReader::readString(serr.get(), *err);

  func.waitForEnd();
  return true;
}

int Process::Exec(const std::string &cmd, const std::string &outf,
                  const std::string &errf) {
  vector<string> argvs;
  Util::split(' ', cmd.c_str(), argvs);
  if (argvs.empty()) {
    return -1;
  }

  int pid = fork();
  if (pid < 0) {
    Logger::Error("Unable to fork: %d %s", errno,
                  Util::safe_strerror(errno).c_str());
    return 0;
  }
  if (pid == 0) {
    signal(SIGTSTP,SIG_IGN);

    swap_fd(outf, stdout);
    swap_fd(errf, stderr);

    int count = argvs.size();
    char **argv = (char**)calloc(count + 1, sizeof(char*));
    for (int i = 0; i < count; i++) {
      argv[i] = (char*)argvs[i].c_str();
    }
    argv[count] = NULL;

    execvp(argv[0], argv);
    _exit(-1);
  }
  int status = -1;
  wait(&status);
  return status;
}

int Process::Exec(const char *path, const char *argv[], int *fdin, int *fdout,
                  int *fderr) {
  CPipe pipein, pipeout, pipeerr;
  if (!pipein.open() || !pipeout.open() || !pipeerr.open()) {
    return 0;
  }

  int pid = fork();
  if (pid < 0) {
    Logger::Error("Unable to fork: %d %s", errno,
                  Util::safe_strerror(errno).c_str());
    return 0;
  }
  if (pid == 0) {
    /**
     * I don't know why, but things work alot better if this process ignores
     * the tstp signal (ctrl-Z). If not, it locks up if you hit ctrl-Z then
     * "bg" the program.
     */
    signal(SIGTSTP,SIG_IGN);

    if (!pipein.dupOut2(fileno(stdin)) || !pipeout.dupIn2(fileno(stdout)) ||
        !pipeerr.dupIn2(fileno(stderr))) {
      return 0;
    }

    pipeout.close(); pipeerr.close(); pipein.close();

    const char *argvnull[2] = {"", NULL};
    execvp(path, const_cast<char**>(argv ? argv : argvnull));
    return 0;
  }
  if (fdout) *fdout = pipeout.detachOut();
  if (fderr) *fderr = pipeerr.detachOut();
  if (fdin)  *fdin  = pipein.detachIn();
  return pid;
}

/**
 * Copied from http://www-theorie.physik.unizh.ch/~dpotter/howto/daemonize
 */
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
void Process::Daemonize(const char *stdoutFile /* = "/dev/null" */,
                        const char *stderrFile /* = "/dev/null" */) {
  pid_t pid, sid;

  /* already a daemon */
  if (getppid() == 1) return;

  /* Fork off the parent process */
  pid = fork();
  if (pid < 0) {
    exit(EXIT_FAILURE);
  }
  /* If we got a good PID, then we can exit the parent process. */
  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }

  /* At this point we are executing as the child process */

  /* Change the file mode mask */
  umask(0);

  /* Create a new SID for the child process */
  sid = setsid();
  if (sid < 0) {
    exit(EXIT_FAILURE);
  }

  /* Change the current working directory.  This prevents the current
     directory from being locked; hence not being able to remove it. */
  if ((chdir("/")) < 0) {
    exit(EXIT_FAILURE);
  }

  /* Redirect standard files to /dev/null */
  freopen("/dev/null", "r", stdin);
  if (stdoutFile && *stdoutFile) {
    freopen(stdoutFile, "a", stdout);
  } else {
    freopen("/dev/null", "w", stdout);
  }
  if (stderrFile && *stderrFile) {
    freopen(stderrFile, "a", stderr);
  } else {
    freopen("/dev/null", "w", stderr);
  }
}

///////////////////////////////////////////////////////////////////////////////
// /proc/* parsing functions

pid_t Process::GetProcessId(const std::string &cmd,
                            bool matchAll /* = false */) {
  std::vector<pid_t> pids;
  GetProcessId(cmd, pids, matchAll);
  return pids.empty() ? 0 : pids[0];
}

void Process::GetProcessId(const std::string &cmd, std::vector<pid_t> &pids,
                           bool matchAll /* = false */) {
  const char *argv[] = {"", "/proc", "-regex", "/proc/[0-9]+/cmdline", NULL};
  string out;
  Exec("find", argv, NULL, out);

  vector<string> files;
  Util::split('\n', out.c_str(), files, true);

  string ccmd = cmd;
  if (!matchAll) {
    size_t pos = ccmd.find(' ');
    if (pos != string::npos) {
      ccmd = ccmd.substr(0, pos);
    }
    pos = ccmd.rfind('/');
    if (pos != string::npos) {
      ccmd = ccmd.substr(pos + 1);
    }
  } else {
    ccmd += " ";
  }

  for (unsigned int i = 0; i < files.size(); i++) {
    string &filename = files[i];

    FILE * f = fopen(filename.c_str(), "r");
    if (f) {
      string cmdline;
      FileReader::readString(f, cmdline);
      fclose(f);
      string converted;
      if (matchAll) {
        for (unsigned int i = 0; i < cmdline.size(); i++) {
          char ch = cmdline[i];
          converted += ch ? ch : ' ';
        }
      } else {
        converted = cmdline;
        size_t pos = converted.find('\0');
        if (pos != string::npos) {
          converted = converted.substr(0, pos);
        }
        pos = converted.rfind('/');
        if (pos != string::npos) {
          converted = converted.substr(pos + 1);
        }
      }

      if (converted == ccmd && filename.find("/proc/") == 0) {
        long long pid = atoll(filename.c_str() + strlen("/proc/"));
        if (pid) {
          pids.push_back(pid);
        }
      }
    }
  }
}

int Process::GetProcessRSS(pid_t pid) {
  string name = "/proc/" + boost::lexical_cast<string>((long long)pid) +
    "/status";

  string status;
  FILE * f = fopen(name.c_str(), "r");
  if (f) {
    FileReader::readString(f, status);
    fclose(f);
  }

  vector<string> lines;
  Util::split('\n', status.c_str(), lines, true);
  for (unsigned int i = 0; i < lines.size(); i++) {
    string &line = lines[i];
    if (line.find("VmRSS:") == 0) {
      for (unsigned int j = strlen("VmRSS:"); j < line.size(); j++) {
        if (line[j] != ' ') {
          long long mem = atoll(line.c_str() + j);
          return mem/1024;
        }
      }
    }
  }

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
}

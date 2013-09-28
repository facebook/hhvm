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

#include <pwd.h>
#include "hphp/util/cronolog.h"
#include "hphp/util/util.h"

/* Default permissions for files and directories that are created */

#ifndef FILE_MODE
#define FILE_MODE       ( S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH )
#endif

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

/* Open a new log file: determine the start of the current
 * period, generate the log file name from the fileTemplate,
 * determine the end of the period and open the new log file.
 *
 * Returns the file descriptor of the new log file and also sets the
 * name of the file and the start time of the next period via pointers
 * supplied.
 */
static FILE *new_log_file(const char *fileTemplate, const char *linkname,
                          mode_t linktype, const char *prevlinkname,
                          PERIODICITY periodicity, int period_multiple,
                          int period_delay, char *pfilename,
                          size_t pfilename_len, time_t time_now,
                          time_t *pnext_period) {
  time_t start_of_period;
  struct tm   *tm;
  int log_fd;

  start_of_period = start_of_this_period(time_now, periodicity,
                                         period_multiple);
  tm = localtime(&start_of_period);
  strftime(pfilename, pfilename_len, fileTemplate, tm);
  *pnext_period = start_of_next_period(start_of_period, periodicity,
                                       period_multiple) + period_delay;

  CRONO_DEBUG(("%s (%d): using log file \"%s\" from %s (%d) until %s (%d) "
        "(for %d secs)\n",
        timestamp(time_now), time_now, pfilename,
        timestamp(start_of_period), start_of_period,
        timestamp(*pnext_period), *pnext_period,
        *pnext_period - time_now));

  log_fd = open(pfilename, O_WRONLY|O_CREAT|O_APPEND, FILE_MODE);

#ifndef DONT_CREATE_SUBDIRS
  if ((log_fd < 0) && (errno == ENOENT)) {
    create_subdirs(pfilename);
    log_fd = open(pfilename, O_WRONLY|O_CREAT|O_APPEND, FILE_MODE);
  }
#endif

  if (log_fd < 0) {
    perror(pfilename);
    return nullptr;
  }

  if (linkname) {
    /* Create a relative symlink to logs under linkname's directory */
    std::string dir = Util::safe_dirname(linkname);
    if (dir != "/") {
      dir.append("/");
    }
    std::string filename;
    if (!strncmp(pfilename, dir.c_str(), dir.length())) {
      filename = pfilename + dir.length();
    } else {
      filename = pfilename;
    }

    create_link(filename.c_str(), linkname, linktype, prevlinkname);
  }
  return fdopen(log_fd, "a");
}

void Cronolog::setPeriodicity() {
  if (m_periodicity == UNKNOWN) {
    m_periodicity = determine_periodicity((char *)m_template.c_str());
  }
}

FILE *Cronolog::getOutputFile() {
  if (m_template.empty()) return m_file;

  time_t time_now = time(nullptr) + m_timeOffset;
  /* If the current period has not finished and there is a log file, use it */
  if ((time_now < m_nextPeriod) && (m_file)) return m_file;

  /* We need to open a new file under a mutex. */
  {
    Lock lock(m_mutex);
    if ((time_now >= m_nextPeriod)) {
      /* the current period has finished */

      /* We cannot close m_file because there may be other threads still
       * writing to it. We save m_file in m_prevFile and leave it open for
       * an entire period. We simply assume that by the end of the delay
       * no threads should be still referencing m_prevFile and we can safely
       * close it.
       */
      if (m_prevFile) fclose(m_prevFile);
      m_prevFile = m_file;
      m_file = nullptr;
    }

    /* If there is no log file open then open a new one. */
    if (m_file == nullptr) {
      const char *linkname = m_linkName.empty() ? nullptr : m_linkName.c_str();
      m_file = new_log_file(m_template.c_str(), linkname, S_IFLNK,
                            m_prevLinkName, m_periodicity, m_periodMultiple,
                            m_periodDelay, m_fileName, sizeof(m_fileName),
                            time_now, &m_nextPeriod);
    }
  }
  return m_file;
}

void Cronolog::changeOwner(const string &username, const string &symlink) {
  if (username.empty() || symlink.empty()) {
    return;
  }

  int username_length = sysconf(_SC_GETPW_R_SIZE_MAX);
  if (username_length == -1) {
    username_length = 512;
  }

  struct passwd user_info, *user_infop;
  std::vector<char> username_buf(username_length);

  if (getpwnam_r(username.c_str(), &user_info, &username_buf[0],
                 username_length, &user_infop)) {
    // invalid user
    return;
  }

  if (lchown(symlink.c_str(), user_info.pw_uid, -1) < 0) {
    fprintf(stderr, "Unable to chmod %s\n", symlink.c_str());
  }

  // using chown() isn't portable if it is a symlink
  int fd = open(symlink.c_str(), O_RDONLY | O_NONBLOCK | O_NOCTTY);
  int success = (fd >= 0 ? fchown(fd, user_info.pw_uid, -1) : -1);

  if (fd >= 0) {
    close(fd);
  }

  if (success < 0) {
    fprintf(stderr, "Unable to chmod %s\n", symlink.c_str());
  }
}

///////////////////////////////////////////////////////////////////////////////
}

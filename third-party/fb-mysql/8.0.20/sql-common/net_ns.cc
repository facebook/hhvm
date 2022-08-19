/*
   Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "sql-common/net_ns.h"

#ifdef HAVE_SETNS

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sched.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <map>
#include <mutex>
#include <unordered_map>

#include "my_dbug.h"
#include "my_sys.h"
#include "mysql/components/services/log_builtins.h"
#include "mysqld_error.h"

#ifdef MYSQL_SERVER
using namespaces_registry_t = std::map<std::string, int>;

static namespaces_registry_t opened_namespaces;

/*
  This mutex is to protect access to the opened_namespaces object.
*/
static std::mutex lock;

#else
/*
  fd for opened file corresponding to a network namespace.
  Since the mysql program and other similar programs use only one connection
  to a server it is ok to store a file descriptor for opened network namespace
  in primitive variable. Also it is worth to note that the mysql program
  and similar work with connection to a server in single threaded mode so
  data protection by mutex not required.
*/
static int ns_fd = -1;
#endif

static int original_ns_fd = -1;

/**
  Open a network namespace specified by function argument.

  @param network_namespace  a name of the network namespace to open
  @param[out] fd  file descriptor corresponding to
                  the opened network namespace

  @return false on success, true on failure
*/
static bool open_network_namespace(const std::string &network_namespace,
                                   int *fd) {
#ifdef MYSQL_SERVER
  std::lock_guard<std::mutex> lck_guard(lock);
  auto iter = opened_namespaces.find(network_namespace);
  if (iter != opened_namespaces.end()) {
    *fd = iter->second;
    return false;
  }
#endif
  char path_to_ns_file[PATH_MAX];
  int requested_len = snprintf(path_to_ns_file, sizeof(path_to_ns_file),
                               "/var/run/netns/%s", network_namespace.c_str());
  if (requested_len + 1 > PATH_MAX) {
#ifdef MYSQL_SERVER
    LogErr(ERROR_LEVEL, ER_NETWORK_NAMESPACE_FILE_PATH_TOO_LONG, requested_len,
           PATH_MAX);
#endif
    return true;
  }
  *fd = open(path_to_ns_file, O_RDONLY);

  if (*fd == -1) {
#ifdef MYSQL_SERVER
    if (errno == ENOENT)
      LogErr(ERROR_LEVEL, ER_UNKNOWN_NETWORK_NAMESPACE,
             network_namespace.c_str());
    else {
      char errbuf[PATH_MAX];
      LogErr(ERROR_LEVEL, ER_SERVER_CANT_OPEN_FILE, path_to_ns_file, errno,
             my_strerror(errbuf, sizeof(errbuf), errno));
    }
#endif
    return true;
  }

#ifdef MYSQL_SERVER
  opened_namespaces.insert(std::make_pair(network_namespace, *fd));
#else
  ns_fd = *fd;
#endif

  return false;
}

/**
  Remember a file descriptor corresponding to the current
  active network namespace.

  @param[out] orig_ns_fd  file descriptor corresponding
              to a network namespace used to be active before

  @return false on success, true on failure
*/
static bool save_original_network_namespace(int *orig_ns_fd) {
  static const char *path_to_current_ns_file = "/proc/self/ns/net";

  int fd = open(path_to_current_ns_file, O_RDONLY);

  if (fd == -1) {
#ifdef MYSQL_SERVER
    char errbuf[MYSYS_STRERROR_SIZE];
    LogErr(ERROR_LEVEL, ER_SERVER_CANT_OPEN_FILE, path_to_current_ns_file,
           errno, my_strerror(errbuf, sizeof(errbuf), errno));
#endif
    return true;
  }

  *orig_ns_fd = fd;
  return false;
}

bool set_network_namespace(const std::string &network_namespace) {
  int fd;

  if (original_ns_fd == -1 && save_original_network_namespace(&original_ns_fd))
    return true;

  if (open_network_namespace(network_namespace, &fd)) return true;

  if (setns(fd, CLONE_NEWNET) != 0) {
#ifdef MYSQL_SERVER
    char errbuf[MYSYS_STRERROR_SIZE];

    LogErr(ERROR_LEVEL, ER_SETNS_FAILED,
           my_strerror(errbuf, sizeof(errbuf), errno));
#endif
    close(fd);

    return true;
  }

  return false;
}

bool restore_original_network_namespace() {
  DBUG_ASSERT(original_ns_fd != -1);

  if (setns(original_ns_fd, CLONE_NEWNET) != 0) {
#ifdef MYSQL_SERVER
    char errbuf[MYSYS_STRERROR_SIZE];

    LogErr(ERROR_LEVEL, ER_SETNS_FAILED,
           my_strerror(errbuf, sizeof(errbuf), errno));
#endif
    return true;
  }
  return false;
}

void release_network_namespace_resources() {
#ifdef MYSQL_SERVER
  std::lock_guard<std::mutex> lck_guard(lock);

  for (const auto &element : opened_namespaces) {
    (void)close(element.second);
  }
  opened_namespaces.clear();
#else
  if (ns_fd > -1) {
    close(ns_fd);
    ns_fd = -1;
  }
#endif
  if (original_ns_fd > -1) {
    close(original_ns_fd);
    original_ns_fd = -1;
  }
}

#endif

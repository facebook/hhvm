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

#include "hphp/runtime/server/cert-reloader.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/extended-logger.h"
#include "hphp/runtime/base/file-util.h"
#include <sys/types.h>
#include <sys/stat.h>

#include <folly/portability/Fcntl.h>

namespace HPHP {

const std::string CertReloader::crt_ext = ".crt";
const std::string CertReloader::key_ext = ".key";

/**
 * Given a default SSL config, SSL_CTX, and certificate path, load certs.
 */
void CertReloader::load(const std::string &cert_dir,
                                CertHandlerFn certHandlerFn) {
  auto path = cert_dir;
  // Ensure path ends with '/'. This helps our pruning later.
  if (path.size() > 0 && path[path.size() - 1] != '/') {
    path.append("/");
  }

  std::vector<CertKeyPair> paths;
  find_paths(path, paths);
  certHandlerFn(paths);
}

bool CertReloader::fileIsValid(const std::string &filename) {
  if (filename.empty()) {
    return false;
  }
  int fd = open(filename.c_str(), O_RDONLY);
  if (fd >= 0) {
    close(fd);
    return true;
  }
  return false;
}

void CertReloader::find_paths(
    const std::string &path,
    std::vector<CertKeyPair> &cert_key_paths) {

  // Iterate through all files in the cert directory.
  std::vector<std::string> crt_dir_files;
  std::unordered_set<std::string> crt_files;
  std::unordered_set<std::string> key_files;
  FileUtil::find(crt_dir_files, "/", path, /* php */ false);

  for (auto it = crt_dir_files.begin(); it != crt_dir_files.end(); ++it) {
    size_t filename_len = it->size() - path.size();
    if (ends_with(*it, crt_ext) &&
        *it != Cfg::Server::SSLCertificateFile) {
      std::string name = it->substr(path.size(), filename_len - crt_ext.size());
      crt_files.insert(name);
    } else if (ends_with(*it, key_ext) &&
        *it != Cfg::Server::SSLCertificateKeyFile) {
      std::string name = it->substr(path.size(), filename_len - key_ext.size());
      key_files.insert(name);
    }
  }

  // Intersect key_files and crt_files to find valid pairs.
  std::unordered_set<ino_t> crt_inodes;
  for (auto name: key_files) {
    auto crt_file_it = crt_files.find(name);
    if (crt_file_it == crt_files.end()) {
      continue;
    }
    struct stat statbuf;
    std::string crt_path = folly::to<std::string>(path, name, crt_ext);
    std::string key_path = folly::to<std::string>(path, name, key_ext);

    int rc = stat(crt_path.c_str(), &statbuf);
    if (rc == 0 && !crt_inodes.insert(statbuf.st_ino).second) {
      continue;
    } else if (rc != 0) {
      Logger::Warning("Stat '%s' failed with errno=%d", crt_path.c_str(),
                      errno);
    }
    if (!fileIsValid(key_path) || !fileIsValid(crt_path)) {
      continue;
    }
    cert_key_paths.push_back({crt_path, key_path});
  }
}

bool CertReloader::ends_with(const std::string &s,
                                     const std::string &end) {
  if (s.size() > end.size()) {
    return std::equal(s.begin() + s.size() - end.size(), s.end(), end.begin());
  }
  return false;
}
}

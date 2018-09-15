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

#include "hphp/runtime/server/server-name-indication.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/extended-logger.h"
#include "hphp/runtime/base/file-util.h"
#include <openssl/ssl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <folly/portability/Fcntl.h>

namespace HPHP {

hphp_string_map<SSL_CTX *> ServerNameIndication::s_sn_ctxd_map;
std::string ServerNameIndication::s_path;
ServerNameIndication::CertHandlerFn ServerNameIndication::s_certHandlerFn;

const std::string ServerNameIndication::crt_ext = ".crt";
const std::string ServerNameIndication::key_ext = ".key";

/**
 * Given a default SSL config, SSL_CTX, and certificate path, load certs.
 */
void ServerNameIndication::load(const std::string &cert_dir,
                                CertHandlerFn certHandlerFn) {
  // We use these later to dynamically load certs, so make copies.
  s_path = cert_dir;
  s_certHandlerFn = certHandlerFn;

  // Ensure path ends with '/'. This helps our pruning later.
  if (s_path.size() > 0 && s_path[s_path.size() - 1] != '/') {
    s_path.append("/");
  }

  std::vector<std::pair<std::string, bool>> server_names;
  find_server_names(s_path, server_names);

  for (auto it = server_names.begin(); it != server_names.end(); ++it) {
    loadFromFile(it->first, it->second, certHandlerFn);
  }
}

bool ServerNameIndication::loadFromFile(const std::string &server_name,
                                        bool duplicate,
                                        CertHandlerFn certHandlerFn) {
  std::string key_file = s_path + server_name + key_ext;
  std::string crt_file = s_path + server_name + crt_ext;

  if (!fileIsValid(key_file) || !fileIsValid(crt_file)) {
    return false;
  }

  return certHandlerFn(server_name, key_file, crt_file, duplicate);
}

void ServerNameIndication::insertSNICtx(const std::string &server_name,
                                        SSL_CTX *ctx) {
  s_sn_ctxd_map.insert(make_pair(server_name, ctx));
}

bool ServerNameIndication::fileIsValid(const std::string &filename) {
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

void ServerNameIndication::find_server_names(
    const std::string &path,
    std::vector<std::pair<std::string, bool>> &server_names) {

  hphp_string_map<bool> crt_files;
  hphp_string_map<bool> key_files;

  // Iterate through all files in the cert directory.
  std::vector<std::string> crt_dir_files;
  FileUtil::find(crt_dir_files, "/", path, /* php */ false);
  for (auto it = crt_dir_files.begin(); it != crt_dir_files.end(); ++it) {
    // Skip default cert and key; we'll fall back to those anyway.
    size_t filename_len = it->size() - path.size();
    if (ends_with(*it, crt_ext) && *it != RuntimeOption::SSLCertificateFile) {
      std::string name = it->substr(path.size(), filename_len - crt_ext.size());
      crt_files.insert(make_pair(name, true));
    } else if (ends_with(*it, key_ext) &&
               *it != RuntimeOption::SSLCertificateKeyFile) {
      std::string name = it->substr(path.size(), filename_len - key_ext.size());
      key_files.insert(make_pair(name, true));
    }
  }

  // Intersect key_files and crt_files to find valid pairs.
  std::unordered_set<ino_t> crt_inodes;
  for (auto it = key_files.begin(); it != key_files.end(); ++it) {
    auto crt_file_it = crt_files.find(it->first);
    if (crt_file_it == crt_files.end()) {
      continue;
    }
    struct stat statbuf;
    std::string crt_path = folly::to<std::string>(path, crt_file_it->first,
                                                  crt_ext);
    int rc = stat(crt_path.c_str(), &statbuf);
    bool dup = false;
    if (rc == 0) {
      dup = !crt_inodes.insert(statbuf.st_ino).second;
    } else {
      Logger::Warning("Stat '%s' failed with errno=%d", crt_path.c_str(),
                      errno);
    }
    server_names.push_back({it->first, dup});
  }
}

bool ServerNameIndication::ends_with(const std::string &s,
                                     const std::string &end) {
  if (s.size() > end.size()) {
    return std::equal(s.begin() + s.size() - end.size(), s.end(), end.begin());
  }
  return false;
}

int ServerNameIndication::callback(void* s, int* /*ad*/, void* /*arg*/) {
  SSL *ssl = (SSL *)s;
  const char *sn_ptr = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);
  if (!sn_ptr) {
    return SSL_TLSEXT_ERR_OK; // No server name; use the default.
  }

  // Calculate the names to search for: fqdn and wildcard.
  std::string fqdn = sn_ptr;
  size_t pos = fqdn.find('.');
  std::string wildcard;
  if (pos != std::string::npos) {
    wildcard = fqdn.substr(pos + 1);
  }

  // Search in memory for matching certificate.
  if (setCTXFromMemory(ssl, wildcard) || setCTXFromMemory(ssl, fqdn)) {
    return SSL_TLSEXT_ERR_OK;
  }

  if (setCTXFromFile(ssl, wildcard) || setCTXFromFile(ssl, fqdn)) {
    return SSL_TLSEXT_ERR_OK;
  }

  // Didn't find a match based on SNI, fallback to default.
  return SSL_TLSEXT_ERR_OK;
}

bool ServerNameIndication::setCTXFromMemory(SSL *ssl, const std::string &name) {
  if (!ssl || name.empty()) {
    return false;
  }
  auto it = s_sn_ctxd_map.find(name);
  if (it != s_sn_ctxd_map.end()) {
    SSL_CTX *ctx = it->second;
    if (ctx && ctx == SSL_set_SSL_CTX(ssl, ctx)) {
      return true;
    }
  }
  return false;
}

bool ServerNameIndication::setCTXFromFile(SSL *ssl, const std::string &name) {
  return s_certHandlerFn &&
    loadFromFile(name, false, s_certHandlerFn) && setCTXFromMemory(ssl, name);
  return false;
}

}

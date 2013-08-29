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

#include "hphp/runtime/server/server-name-indication.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/extended-logger.h"
#include "hphp/util/util.h"
#include "openssl/ssl.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace HPHP {

hphp_string_map<SSL_CTX *> ServerNameIndication::sn_ctxd_map;
std::string ServerNameIndication::path;
struct ssl_config ServerNameIndication::config;

const std::string ServerNameIndication::crt_ext = ".crt";
const std::string ServerNameIndication::key_ext = ".key";

/**
 * Given a default SSL config, SSL_CTX, and certificate path, load certs.
 */
void ServerNameIndication::load(void *ctx, const struct ssl_config &cfg,
                                const std::string &cert_dir) {

  if (!ctx) {
    return;
  }

  // We use these later to dynamically load certs, so make copies.
  path = cert_dir;
  config = cfg;

  // Ensure path ends with '/'. This helps our pruning later.
  if (path.size() > 0 && path[path.size() - 1] != '/') {
    path.append("/");
  }

  vector<std::string> server_names;
  find_server_names(path, server_names);

  for (vector<std::string>::iterator it = server_names.begin();
      it != server_names.end();
      ++it) {
    loadFromFile(*it);
  }

  // Register our per-request server name indication callback.
  // We register our callback even if there's no additional certs so that
  // a cert added in the future will get picked up without a restart.
  SSL_CTX_set_tlsext_servername_callback(
      (SSL_CTX*)ctx,
      ServerNameIndication::callback);
}

bool ServerNameIndication::loadFromFile(const std::string &server_name) {
  std::string key_file = path + server_name + key_ext;
  std::string crt_file = path + server_name + crt_ext;
  struct ssl_config tmp_config = config;

  if (!fileIsValid(key_file) || !fileIsValid(crt_file)) {
    return false;
  }

  // Create an SSL_CTX for this cert pair.
  tmp_config.cert_file = (char *)(crt_file.c_str());
  tmp_config.pk_file = (char *)(key_file.c_str());
  SSL_CTX *tmp_ctx = (SSL_CTX*)evhttp_init_openssl(&tmp_config);
  if (tmp_ctx) {
    sn_ctxd_map.insert(make_pair(server_name, tmp_ctx));
    return true;
  }
  return false;
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
    vector<std::string> &server_names) {

  hphp_string_map<bool> crt_files;
  hphp_string_map<bool> key_files;

  // Iterate through all files in the cert directory.
  vector<std::string> crt_dir_files;
  Util::find(crt_dir_files, "/", path.c_str(), /* php */ false);
  for (vector<std::string>::iterator it = crt_dir_files.begin();
       it != crt_dir_files.end();
       ++it) {

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
  for (hphp_string_map<bool>::iterator it = key_files.begin();
      it != key_files.end();
      ++it) {
    if (crt_files.find(it->first) == crt_files.end()) {
      continue;
    }
    server_names.push_back(it->first);
  }
}

bool ServerNameIndication::ends_with(const std::string &s,
                                     const std::string &end) {
  if (s.size() > end.size()) {
    return std::equal(s.begin() + s.size() - end.size(), s.end(), end.begin());
  }
  return false;
}

int ServerNameIndication::callback(void *s, int *ad, void *arg) {
  SSL *ssl = (SSL *)s;
  const char *sn_ptr = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);
  if (!sn_ptr) {
    return SSL_TLSEXT_ERR_OK; // No server name; use the default.
  }

  // Calculate the names to search for: fqdn and wildcard.
  std::string fqdn = sn_ptr;
  size_t pos = fqdn.find('.');
  std::string wildcard;
  if (pos != string::npos) {
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
  hphp_string_map<SSL_CTX *>::iterator it = sn_ctxd_map.find(name);
  if (it != sn_ctxd_map.end()) {
    SSL_CTX *ctx = it->second;
    if (ctx && ctx == SSL_set_SSL_CTX(ssl, ctx)) {
      return true;
    }
  }
  return false;
}

bool ServerNameIndication::setCTXFromFile(SSL *ssl, const std::string &name) {
  return loadFromFile(name) && setCTXFromMemory(ssl, name);
}

}

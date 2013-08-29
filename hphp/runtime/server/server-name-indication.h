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

#ifndef incl_HPHP_SERVER_NAME_INDICATION_H_
#define incl_HPHP_SERVER_NAME_INDICATION_H_

#include "hphp/util/base.h"
#include <evhttp.h>
#include "openssl/ssl.h"

namespace HPHP {

class ServerNameIndication {
  static hphp_string_map<SSL_CTX *> sn_ctxd_map;
  static const std::string crt_ext;
  static const std::string key_ext;
  static std::string path;
  static struct ssl_config config;

public:
  static void load(void *ctx, const struct ssl_config &config,
                   const std::string &cert_dir);
  static int callback(void *s, int *ad, void *arg);

private:
  static bool setCTXFromMemory(SSL*, const string&);
  static bool setCTXFromFile(SSL*, const string&);
  static void find_server_names(const std::string &, vector<std::string> &);
  static bool ends_with(const std::string &, const std::string &);
  static bool loadFromFile(const std::string &);
  static bool fileIsValid(const std::string &);
};

}

#endif // incl_HPHP_SERVER_NAME_INDICATION_H_

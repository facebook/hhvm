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
#include <openssl/ssl.h>

namespace HPHP {

class ServerNameIndication {
public:

  /**
   * The certificate handler function takes the "name" of the server and
   * the paths to the key and certificate.  It should load the keypair,
   * and if valid, add it to the server's SNI map (either via insertSNICtx
   * or it's own structure).
   *
   * Returns true if the cert was added
   */
  typedef std::function<bool(const std::string&, const std::string&,
                             const std::string&)> CertHanlderFn;

  /**
   * Loads all valid key pairs in cert_dir and invokes the handler.
   * Both the dir and the handler are sticky for use in loadFromFile()
   * and the default callback below.
   */
  static void load(const std::string &cert_dir, CertHanlderFn certHandler);

  /**
   * Loads a single key pair with the given name.  Must have called load()
   * previously, which sets the search path.  Handler behaves similarly to
   * load()
   *
   */
  static bool loadFromFile(const std::string &name, CertHanlderFn certHandler);

  /**
   * Inserts a mapping from name:ctx in the global map used in the
   * provided callback.
   */
  static void insertSNICtx(const std::string& name, SSL_CTX* ctx);

  /**
   * SNI callback which can be used with SSL_CTX_set_tlsext_servername_callback
   */
  static int callback(void *s, int *ad, void *arg);

private:
  static hphp_string_map<SSL_CTX *> s_sn_ctxd_map;
  static const std::string crt_ext;
  static const std::string key_ext;
  static std::string s_path;
  static CertHanlderFn s_certHandlerFn;

  static bool setCTXFromMemory(SSL*, const string&);
  static bool setCTXFromFile(SSL*, const string&);
  static void find_server_names(const std::string &, vector<std::string> &);
  static bool ends_with(const std::string &, const std::string &);
  static bool fileIsValid(const std::string &);
};

}

#endif // incl_HPHP_SERVER_NAME_INDICATION_H_

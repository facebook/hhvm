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

#ifndef incl_HPHP_CERT_RELOADER_H_
#define incl_HPHP_CERT_RELOADER_H_

#include <functional>
#include <string>
#include <vector>

#include "hphp/util/hash-map.h"

namespace HPHP {

struct CertKeyPair {
  std::string certPath;
  std::string keyPath;
};

struct CertReloader {

  /**
   * The certificate handler function takes the paths to the key and certificate
   */
  using CertHandlerFn = std::function<void(
      const std::vector<CertKeyPair>&)>;

  /**
   * Loads all valid cert+key path pairs in cert_dir and invokes the handler.
   */
  static void load(const std::string &cert_dir, CertHandlerFn certHandler);


private:
  static const std::string crt_ext;
  static const std::string key_ext;

  static void find_paths(const std::string &, std::vector<CertKeyPair> &);
  static bool ends_with(const std::string &, const std::string &);
  static bool fileIsValid(const std::string &);
};

}

#endif // incl_HPHP_CERT_RELOADER_H_

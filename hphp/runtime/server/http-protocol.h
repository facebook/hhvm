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

#pragma once

#include "hphp/runtime/server/transport.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Array;
struct RequestURI;
struct StringBuffer;
struct Variant;
struct VirtualHost;

struct HttpProtocol {
  static const VirtualHost *GetVirtualHost(Transport *transport);
  static void PrepareSystemVariables(Transport *transport,
                                     const RequestURI &r);
  static void PrepareRequestVariables(Array& request,
                                      const Array& get,
                                      const Array& post,
                                      const Array& cookie);
  static void PrepareGetVariable(Array& get,
                                 const RequestURI &r);
  static void PreparePostVariables(Array& post,
                                   Variant& raw_post,
                                   Array& files,
                                   Transport* transport,
                                   const RequestURI& r);
  static bool PrepareCookieVariable(Array& cookie,
                                    Transport *transport);
  static void PrepareServerVariable(Array& server,
                                    Transport *transport,
                                    const RequestURI &r,
                                    const VirtualHost *vhost);

  static bool ProxyRequest(Transport *transport, bool force,
                           const std::string &url, int &code,
                           std::string &error, StringBuffer &response,
                           HeaderMap *extraHeaders = nullptr);

  static std::string RecordRequest(Transport *transport);
  static void ClearRecord(bool success, const std::string &tmpfile);

  static void DecodeParameters(Array& variables,
                               const char *data,
                               size_t size,
                               bool post = false);
  static void DecodeRfc1867(Transport* transport,
                            Array& post,
                            Array& files,
                            size_t contentLength,
                            const void*& data,
                            size_t& size,
                            std::string boundary);
  static void DecodeCookies(Array& variables, char *data);
  static bool IsRfc1867(std::string contentType, std::string &boundary);

  static const char *GetReasonString(int code);
};

///////////////////////////////////////////////////////////////////////////////
}


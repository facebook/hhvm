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

#ifndef incl_HPHP_HTTP_PROTOCOL_H_
#define incl_HPHP_HTTP_PROTOCOL_H_

#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/server/transport.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class RequestURI;
class SourceRootInfo;
class VirtualHost;
class StringBuffer;

class HttpProtocol {
public:
  static const VirtualHost *GetVirtualHost(Transport *transport);
  static void PrepareSystemVariables(Transport *transport,
                                     const RequestURI &r,
                                     const SourceRootInfo &sri);
  static void StartRequest();
  static void PrepareEnv(Variant& env,
                         Transport *transport);
  static void PrepareRequestVariables(Variant& request,
                                      Variant& get,
                                      Variant& post,
                                      Variant& raw_post,
                                      Variant& files,
                                      Variant& cookie,
                                      Transport *transport,
                                      const RequestURI &r);
  static void PrepareGetVariable(Variant& get,
                                 const RequestURI &r);
  static void PreparePostVariables(Variant& post,
                                   Variant& raw_post,
                                   Variant& files,
                                   Transport *transport);
  static bool PrepareCookieVariable(Variant& cookie,
                                    Transport *transport);
  static void PrepareServerVariable(Variant& server,
                                    Transport *transport,
                                    const RequestURI &r,
                                    const SourceRootInfo &sri,
                                    const VirtualHost *vhost);
  static void CopyHeaderVariables(Variant& server,
                                  const HeaderMap& headers);
  static void CopyServerInfo(Variant& server,
                             Transport *transport,
                             const VirtualHost *vhost);
  static void CopyRemoteInfo(Variant& server,
                             Transport *transport);
  static void CopyAuthInfo(Variant& server,
                           Transport *transport);
  static void CopyPathInfo(Variant& server,
                           Transport *transport,
                           const RequestURI &r,
                           const VirtualHost *vhost);

  static bool ProxyRequest(Transport *transport, bool force,
                           const std::string &url, int &code,
                           std::string &error, StringBuffer &response,
                           HeaderMap *extraHeaders = nullptr);

  static std::string RecordRequest(Transport *transport);
  static void ClearRecord(bool success, const std::string &tmpfile);

  static void DecodeParameters(Variant &variables,
                               const char *data,
                               int size,
                               bool post = false);
  static void DecodeRfc1867(Transport *transport,
                            Variant &post,
                            Variant &files,
                            int contentLength,
                            const void *&data,
                            int &size,
                            std::string boundary);
  static void DecodeCookies(Variant &variables, char *data);
  static bool IsRfc1867(const std::string contentType, std::string &boundary);

  static const char *GetReasonString(int code);

private:
  static void CopyParams(Variant &dest, Variant &src);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_HTTP_PROTOCOL_H_

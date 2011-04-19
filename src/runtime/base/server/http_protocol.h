/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef __HPHP_HTTP_PROTOCOL_H__
#define __HPHP_HTTP_PROTOCOL_H__

#include <runtime/base/complex_types.h>
#include <runtime/base/server/transport.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class RequestURI;
class SourceRootInfo;
class VirtualHost;
class StringBuffer;

class HttpProtocol {
public:
  static const VirtualHost *GetVirtualHost(Transport *transport);
  static void PrepareSystemVariables(Transport *transport, const RequestURI &r,
                                     const SourceRootInfo &sourceRootInfo);
  static bool ProxyRequest(Transport *transport, bool force,
                           const std::string &url, int &code,
                           std::string &error, StringBuffer &response,
                           HeaderMap *extraHeaders = NULL);

  static std::string RecordRequest(Transport *transport);
  static void ClearRecord(bool success, const std::string &tmpfile);

  static void DecodeParameters(Variant &variables, const char *data, int size,
                               bool post = false);
  static void DecodeRfc1867(Transport *transport,
                            Variant &post, Variant &files, int contentLength,
                            const void *&data, int &size,
                            std::string boundary);
  static void DecodeCookies(Variant &variables, char *data);
  static bool IsRfc1867(const std::string contentType, std::string &boundary);

  static const char *GetReasonString(int code);

private:
  static void CopyParams(Variant &dest, Variant &src);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_HTTP_PROTOCOL_H__

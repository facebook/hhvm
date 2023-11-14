/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/ext/extension.h"

namespace HPHP {

  const int64_t k_PSFS_ERR_FATAL = 0;
  const int64_t k_PSFS_FEED_ME = 1;
  const int64_t k_PSFS_FLAG_FLUSH_CLOSE = 2;
  const int64_t k_PSFS_FLAG_FLUSH_INC = 1;
  const int64_t k_PSFS_FLAG_NORMAL = 0;
  const int64_t k_PSFS_PASS_ON = 2;

  const int64_t k_STREAM_CLIENT_CONNECT = 4;
  const int64_t k_STREAM_CLIENT_ASYNC_CONNECT = 2;
  const int64_t k_STREAM_CLIENT_PERSISTENT = 1;
  const int64_t k_STREAM_META_TOUCH = 1;
  const int64_t k_STREAM_META_OWNER_NAME = 2;
  const int64_t k_STREAM_META_OWNER = 3;
  const int64_t k_STREAM_META_GROUP_NAME = 4;
  const int64_t k_STREAM_META_GROUP = 5;
  const int64_t k_STREAM_META_ACCESS = 6;
  const int64_t k_STREAM_BUFFER_NONE = 0;    /* unbuffered */
  const int64_t k_STREAM_BUFFER_LINE = 1;   /* line buffered */
  const int64_t k_STREAM_BUFFER_FULL = 2;    /* fully buffered */
  const int64_t k_STREAM_SERVER_BIND = 4;
  const int64_t k_STREAM_SERVER_LISTEN = 8;
  // Removed STREAM_CRYPTO_METHOD_SSLv23_CLIENT = 7;
  // Removed STREAM_CRYPTO_METHOD_SSLv23_SERVER = 6;
  // Removed STREAM_CRYPTO_METHOD_SSLv2_CLIENT = 3;
  // Removed STREAM_CRYPTO_METHOD_SSLv2_SERVER = 2;
  // Removed STREAM_CRYPTO_METHOD_SSLv3_CLIENT = 5;
  // Removed STREAM_CRYPTO_METHOD_SSLv3_SERVER = 4;
  const int64_t k_STREAM_CRYPTO_METHOD_TLS_CLIENT = 57;
  const int64_t k_STREAM_CRYPTO_METHOD_TLS_SERVER = 56;
  // Removed STREAM_CRYPTO_METHOD_TLSv1_0_CLIENT = 9;
  // Removed STREAM_CRYPTO_METHOD_TLSv1_0_SERVER = 8;
  // Removed STREAM_CRYPTO_METHOD_TLSv1_1_CLIENT = 17;
  // Removed STREAM_CRYPTO_METHOD_TLSv1_1_SERVER = 16;
  // Removed STREAM_CRYPTO_METHOD_TLSv1_2_CLIENT = 33;
  // Removed STREAM_CRYPTO_METHOD_TLSv1_2_SERVER = 32;
  const int64_t k_STREAM_CRYPTO_METHOD_ANY_CLIENT = 63;
  const int64_t k_STREAM_CRYPTO_METHOD_ANY_SERVER = 62;
  const int64_t k_STREAM_ENFORCE_SAFE_MODE = 4;
  const int64_t k_STREAM_IGNORE_URL = 2;
  const int64_t k_STREAM_IPPROTO_ICMP = 1;
  const int64_t k_STREAM_IPPROTO_IP = 0;
  const int64_t k_STREAM_IPPROTO_RAW = 255;
  const int64_t k_STREAM_IPPROTO_TCP = 6;
  const int64_t k_STREAM_IPPROTO_UDP = 17;
  const int64_t k_STREAM_IS_URL = 1;
  const int64_t k_STREAM_MKDIR_RECURSIVE = 1;
  const int64_t k_STREAM_MUST_SEEK = 16;
  const int64_t k_STREAM_NOTIFY_AUTH_REQUIRED = 3;
  const int64_t k_STREAM_NOTIFY_AUTH_RESULT = 10;
  const int64_t k_STREAM_NOTIFY_COMPLETED = 8;
  const int64_t k_STREAM_NOTIFY_CONNECT = 2;
  const int64_t k_STREAM_NOTIFY_FAILURE = 9;
  const int64_t k_STREAM_NOTIFY_FILE_SIZE_IS = 5;
  const int64_t k_STREAM_NOTIFY_MIME_TYPE_IS = 4;
  const int64_t k_STREAM_NOTIFY_PROGRESS = 7;
  const int64_t k_STREAM_NOTIFY_REDIRECTED = 6;
  const int64_t k_STREAM_NOTIFY_RESOLVE = 1;
  const int64_t k_STREAM_NOTIFY_SEVERITY_ERR = 2;
  const int64_t k_STREAM_NOTIFY_SEVERITY_INFO = 0;
  const int64_t k_STREAM_NOTIFY_SEVERITY_WARN = 1;
  const int64_t k_STREAM_OOB = 1;
  const int64_t k_STREAM_PEEK = 2;
  const int64_t k_STREAM_PF_INET = 2;
  const int64_t k_STREAM_PF_INET6 = 10;
  const int64_t k_STREAM_PF_UNIX = 1;
  const int64_t k_STREAM_REPORT_ERRORS = 8;
  const int64_t k_STREAM_SHUT_RD = 0;
  const int64_t k_STREAM_SHUT_RDWR = 2;
  const int64_t k_STREAM_SHUT_WR = 1;
  const int64_t k_STREAM_SOCK_DGRAM = 2;
  const int64_t k_STREAM_SOCK_RAW = 3;
  const int64_t k_STREAM_SOCK_RDM = 4;
  const int64_t k_STREAM_SOCK_SEQPACKET = 5;
  const int64_t k_STREAM_SOCK_STREAM = 1;
  const int64_t k_STREAM_URL_STAT_LINK = 1;
  const int64_t k_STREAM_URL_STAT_QUIET = 2;
  const int64_t k_STREAM_USE_PATH = 1;

///////////////////////////////////////////////////////////////////////////////
// stream context

struct StreamContext final : ResourceData {
  DECLARE_RESOURCE_ALLOCATION_NO_SWEEP(StreamContext);

  CLASSNAME_IS("stream-context")
  const String& o_getClassNameHook() const override { return classnameof(); }

  StreamContext(const Array& options, const Array& params)
    : m_options(options), m_params(params) {
  }

  static bool validateOptions(const Variant& options);
  void setOption(const String& wrapper, const String& option,
                 const Variant& value);
  void mergeOptions(const Array& options);
  Array getOptions() const;
  static bool validateParams(const Variant& params);
  void mergeParams(const Array& params);
  Array getParams() const;

private:
  static StaticString s_options_key;
  static StaticString s_notification_key;

  Array m_options;
  Array m_params;
};

Variant HHVM_FUNCTION(stream_context_create,
                      const Variant& options = uninit_variant,
                      const Variant& params = uninit_variant);

///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(stream_copy_to_stream,
                      const OptResource& source,
                      const OptResource& dest,
                      int64_t maxlength = -1,
                      int64_t offset = 0);

Variant HHVM_FUNCTION(stream_get_contents,
                      const OptResource& handle,
                      int64_t maxlen = -1,
                      int64_t offset = -1);

///////////////////////////////////////////////////////////////////////////////
}

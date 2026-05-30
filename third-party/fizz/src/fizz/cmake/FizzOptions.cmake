#  Copyright (c) 2018, Facebook, Inc.
#  All rights reserved.

option(FIZZ_CERTIFICATE_USE_OPENSSL_CERT "Compiles Fizz Certificate with OpenSSL dependency" ON)

# Logging backend selection. Default is glog (preserves historical behavior);
# downstream builds can opt in to folly::xlog or fully disable logging.
# Interpolated into fizz-config.h.in as `#define FIZZ_LOGGING_@FIZZ_LOGGING_BACKEND@ 1`.
set(FIZZ_LOGGING_BACKEND "GLOG" CACHE STRING
  "Logging backend. Valid choices: GLOG, XLOG, DISABLED")
set_property(CACHE FIZZ_LOGGING_BACKEND PROPERTY STRINGS GLOG XLOG DISABLED)
if(NOT FIZZ_LOGGING_BACKEND MATCHES "^(GLOG|XLOG|DISABLED)$")
  message(FATAL_ERROR
    "FIZZ_LOGGING_BACKEND must be one of: GLOG, XLOG, DISABLED "
    "(got '${FIZZ_LOGGING_BACKEND}')")
endif()

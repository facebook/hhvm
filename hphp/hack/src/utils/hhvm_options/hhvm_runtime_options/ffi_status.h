// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
#pragma once

enum {
  FFI_STATUS_ENUM_INITIALIZER = -1000,

  // WARNING: Match "hphp/facebook/hacknative/systemlib/hn/main.php".
  FFI_STATUS_THROW_Exception,
  FFI_STATUS_THROW_InvalidArgumentException,

  // WARNING: Match "hphp/facebook/hacknative/systemlib/ext/thrift/hni.php".
  FFI_STATUS_THROW_PDOException,

  // WARNING: Match "hphp/facebook/hacknative/systemlib/fb/initialize.php".
  FFI_STATUS_THROW_CompressionException,
  FFI_STATUS_THROW_CryptoException,
  FFI_STATUS_THROW_CryptoProjectNotFoundException,
  FFI_STATUS_THROW_CryptoUnexpectedException,
  FFI_STATUS_THROW_ManagedCompressionException,

  FFI_STATUS_FATAL = -5,
  FFI_STATUS_ERROR = -4,

  FFI_STATUS_NULL_WITH_WARNING = -3,
  FFI_STATUS_NULL_WITH_NOTICE = -2,
  FFI_STATUS_NULL = -1,

  FFI_STATUS_OK = 0,
};

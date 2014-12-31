<?hh // decl
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

const string PHP_EOL = "\n";

const int PHP_INT_MAX = (1 << 63) - 1;
const int PHP_INT_MIN = -1 << 63;

const int E_ERROR = 1;
const int E_WARNING = 1 << 1;
const int E_PARSE = 1 << 2;
const int E_NOTICE = 1 << 3;
const int E_CORE_ERROR = 1 << 4;
const int E_CORE_WARNING = 1 << 5;
const int E_COMPILE_ERROR = 1 << 6;
const int E_COMPILE_WARNING = 1 << 7;
const int E_USER_ERROR = 1 << 8;
const int E_USER_WARNING = 1 << 9;
const int E_USER_NOTICE = 1 << 10;
const int E_DEPRECATED = 1 << 13;
const int E_USER_DEPRECATED = 1 << 14;
const int E_ALL = (1 << 15) - 1;
const int E_STRICT = 1 << 11;

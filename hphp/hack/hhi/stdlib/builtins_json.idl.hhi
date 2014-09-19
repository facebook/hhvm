<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
const JSON_HEX_TAG = 1;
const JSON_HEX_AMP = 2;
const JSON_HEX_APOS = 4;
const JSON_HEX_QUOT = 8;
const JSON_FORCE_OBJECT = 16;
const JSON_NUMERIC_CHECK = 32;
const JSON_UNESCAPED_SLASHES = 64;
const JSON_PRETTY_PRINT = 128;
const JSON_UNESCAPED_UNICODE = 256;
const JSON_ERROR_NONE = 0;
const JSON_ERROR_DEPTH = 1;
const JSON_ERROR_STATE_MISMATCH = 2;
const JSON_ERROR_CTRL_CHAR = 3;
const JSON_ERROR_SYNTAX = 4;
const JSON_ERROR_UTF8 = 5;
const JSON_OBJECT_AS_ARRAY = 1;
const JSON_BIGINT_AS_STRING = 2;
function json_encode($value, $options = 0, $depth = 512) {}
function json_decode($json, $assoc = false, $depth = 512, $options = 0) {}
function json_last_error() {}
function json_last_error_msg() {}
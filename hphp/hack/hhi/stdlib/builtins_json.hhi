<?hh // decl /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

const int JSON_ERROR_NONE = 0;
const int JSON_ERROR_DEPTH = 1;
const int JSON_ERROR_STATE_MISMATCH = 2;
const int JSON_ERROR_CTRL_CHAR = 3;
const int JSON_ERROR_SYNTAX = 4;
const int JSON_ERROR_UTF8 = 5;
const int JSON_ERROR_RECURSION = 6;
const int JSON_ERROR_INF_OR_NAN = 7;
const int JSON_ERROR_UNSUPPORTED_TYPE = 8;

// json_encode
const int JSON_HEX_TAG                 = 1<<0;
const int JSON_HEX_AMP                 = 1<<1;
const int JSON_HEX_APOS                = 1<<2;
const int JSON_HEX_QUOT                = 1<<3;
const int JSON_FORCE_OBJECT            = 1<<4;
const int JSON_NUMERIC_CHECK           = 1<<5;
const int JSON_UNESCAPED_SLASHES       = 1<<6;
const int JSON_PRETTY_PRINT            = 1<<7;
const int JSON_UNESCAPED_UNICODE       = 1<<8;
const int JSON_PARTIAL_OUTPUT_ON_ERROR = 1<<9;

// json_decode
const int JSON_BIGINT_AS_STRING = 1;

const int JSON_FB_LOOSE = 0;
const int JSON_FB_UNLIMITED = 0;
const int JSON_FB_EXTRA_ESCAPES = 0;
const int JSON_FB_COLLECTIONS = 0;

function json_encode($value, $options = 0, $depth = 512);
function json_decode($json, $assoc = false, $depth = 512, $options = 0);
function json_last_error();
function json_last_error_msg();

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
const int JSON_HEX_TAG = 0;
const int JSON_HEX_AMP = 0;
const int JSON_HEX_APOS = 0;
const int JSON_HEX_QUOT = 0;
const int JSON_FORCE_OBJECT = 0;
const int JSON_NUMERIC_CHECK = 0;
const int JSON_UNESCAPED_SLASHES = 0;
const int JSON_UNESCAPED_UNICODE = 0;
const int JSON_PRETTY_PRINT = 0;
const int JSON_FB_LOOSE = 0;
const int JSON_FB_UNLIMITED = 0;
const int JSON_FB_EXTRA_ESCAPES = 0;
const int JSON_FB_COLLECTIONS = 0;
function json_encode($value, $options = 0) { }
function json_decode($json, $assoc = false, $depth = 512, $options = 0) { }
function json_last_error() { }

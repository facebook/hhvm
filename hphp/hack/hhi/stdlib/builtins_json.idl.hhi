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
const int JSON_HEX_TAG = 1;
const int JSON_HEX_AMP = 2;
const int JSON_HEX_APOS = 4;
const int JSON_HEX_QUOT = 8;
const int JSON_FORCE_OBJECT = 16;
const int JSON_NUMERIC_CHECK = 32;
const int JSON_UNESCAPED_SLASHES = 64;
const int JSON_PRETTY_PRINT = 128;
const int JSON_UNESCAPED_UNICODE = 256;
const int JSON_ERROR_NONE = 0;
const int JSON_ERROR_DEPTH = 1;
const int JSON_ERROR_STATE_MISMATCH = 2;
const int JSON_ERROR_CTRL_CHAR = 3;
const int JSON_ERROR_SYNTAX = 4;
const int JSON_ERROR_UTF8 = 5;
const int JSON_OBJECT_AS_ARRAY = 1;
const int JSON_BIGINT_AS_STRING = 2;
function json_encode(mixed $value, int $options = 0, int $depth = 512): mixed { }
function json_decode(string $json, bool $assoc = false, int $depth = 512, int $options = 0): mixed { }
function json_last_error(): int { }
function json_last_error_msg(): ?string {}
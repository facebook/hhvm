<?hh /* -*- php -*- */
/**
* Copyright (c) 2014, Facebook, Inc.
* All rights reserved.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the 'hack' directory of this source tree.
*
*/

const int FILTER_FLAG_NONE;
const int FILTER_REQUIRE_SCALAR;
const int FILTER_REQUIRE_ARRAY;
const int FILTER_FORCE_ARRAY;
const int FILTER_NULL_ON_FAILURE;
const int FILTER_VALIDATE_INT;
const int FILTER_VALIDATE_BOOLEAN;
const int FILTER_VALIDATE_FLOAT;
const int FILTER_VALIDATE_REGEXP;
const int FILTER_VALIDATE_URL;
const int FILTER_VALIDATE_EMAIL;
const int FILTER_VALIDATE_IP;
const int FILTER_VALIDATE_MAC;
const int FILTER_DEFAULT;
const int FILTER_UNSAFE_RAW;
const int FILTER_SANITIZE_STRING;
const int FILTER_SANITIZE_STRIPPED;
const int FILTER_SANITIZE_ENCODED;
const int FILTER_SANITIZE_SPECIAL_CHARS;
const int FILTER_SANITIZE_FULL_SPECIAL_CHARS;
const int FILTER_SANITIZE_EMAIL;
const int FILTER_SANITIZE_URL;
const int FILTER_SANITIZE_NUMBER_INT;
const int FILTER_SANITIZE_NUMBER_FLOAT;
const int FILTER_SANITIZE_MAGIC_QUOTES;
const int FILTER_CALLBACK;
const int FILTER_FLAG_ALLOW_OCTAL;
const int FILTER_FLAG_ALLOW_HEX;
const int FILTER_FLAG_STRIP_LOW;
const int FILTER_FLAG_STRIP_HIGH;
const int FILTER_FLAG_ENCODE_LOW;
const int FILTER_FLAG_ENCODE_HIGH;
const int FILTER_FLAG_ENCODE_AMP;
const int FILTER_FLAG_NO_ENCODE_QUOTES;
const int FILTER_FLAG_EMPTY_STRING_NULL;
const int FILTER_FLAG_STRIP_BACKTICK;
const int FILTER_FLAG_ALLOW_FRACTION;
const int FILTER_FLAG_ALLOW_THOUSAND;
const int FILTER_FLAG_ALLOW_SCIENTIFIC;
const int FILTER_FLAG_SCHEME_REQUIRED;
const int FILTER_FLAG_HOST_REQUIRED;
const int FILTER_FLAG_PATH_REQUIRED;
const int FILTER_FLAG_QUERY_REQUIRED;
const int FILTER_FLAG_IPV4;
const int FILTER_FLAG_IPV6;
const int FILTER_FLAG_NO_RES_RANGE;
const int FILTER_FLAG_NO_PRIV_RANGE;

<<__PHPStdLib, __Deprecated('will be removed in a future version of hack')>>
function filter_has_var(int $type, string $variable_name): bool {}
<<__PHPStdLib>>
function filter_id(string $name)[]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib, __Deprecated('will be removed in a future version of hack')>>
function filter_input_array(
  int $type,
  mixed $definition = null,
  bool $add_empty = true,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib, __Deprecated('will be removed in a future version of hack')>>
function filter_input(
  int $type,
  string $variable_name,
  int $filter = FILTER_DEFAULT,
  mixed $options = null,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function filter_list()[]: varray<string> {}
<<__PHPStdLib>>
function filter_var_array(
  Container<mixed> $data,
  mixed $definition = null,
  bool $add_empty = true,
)[]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function filter_var(
  mixed $value,
  int $filter = FILTER_DEFAULT,
  mixed $options = null,
)[]: HH\FIXME\MISSING_RETURN_TYPE {}

<?hh   /* -*- php -*- */
/**
* Copyright (c) 2014, Facebook, Inc.
* All rights reserved.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the 'hack' directory of this source tree.
*
*/

const int FILTER_FLAG_NONE = 0;
const int FILTER_REQUIRE_SCALAR = 0;
const int FILTER_REQUIRE_ARRAY = 0;
const int FILTER_FORCE_ARRAY = 0;
const int FILTER_NULL_ON_FAILURE = 0;
const int FILTER_VALIDATE_INT = 0;
const int FILTER_VALIDATE_BOOLEAN = 0;
const int FILTER_VALIDATE_FLOAT = 0;
const int FILTER_VALIDATE_REGEXP = 0;
const int FILTER_VALIDATE_URL = 0;
const int FILTER_VALIDATE_EMAIL = 0;
const int FILTER_VALIDATE_IP = 0;
const int FILTER_VALIDATE_MAC = 0;
const int FILTER_DEFAULT = 0;
const int FILTER_UNSAFE_RAW = 0;
const int FILTER_SANITIZE_STRING = 0;
const int FILTER_SANITIZE_STRIPPED = 0;
const int FILTER_SANITIZE_ENCODED = 0;
const int FILTER_SANITIZE_SPECIAL_CHARS = 0;
const int FILTER_SANITIZE_FULL_SPECIAL_CHARS = 0;
const int FILTER_SANITIZE_EMAIL = 0;
const int FILTER_SANITIZE_URL = 0;
const int FILTER_SANITIZE_NUMBER_INT = 0;
const int FILTER_SANITIZE_NUMBER_FLOAT = 0;
const int FILTER_SANITIZE_MAGIC_QUOTES = 0;
const int FILTER_CALLBACK = 0;
const int FILTER_FLAG_ALLOW_OCTAL = 0;
const int FILTER_FLAG_ALLOW_HEX = 0;
const int FILTER_FLAG_STRIP_LOW = 0;
const int FILTER_FLAG_STRIP_HIGH = 0;
const int FILTER_FLAG_ENCODE_LOW = 0;
const int FILTER_FLAG_ENCODE_HIGH = 0;
const int FILTER_FLAG_ENCODE_AMP = 0;
const int FILTER_FLAG_NO_ENCODE_QUOTES = 0;
const int FILTER_FLAG_EMPTY_STRING_NULL = 0;
const int FILTER_FLAG_STRIP_BACKTICK = 0;
const int FILTER_FLAG_ALLOW_FRACTION = 0;
const int FILTER_FLAG_ALLOW_THOUSAND = 0;
const int FILTER_FLAG_ALLOW_SCIENTIFIC = 0;
const int FILTER_FLAG_SCHEME_REQUIRED = 0;
const int FILTER_FLAG_HOST_REQUIRED = 0;
const int FILTER_FLAG_PATH_REQUIRED = 0;
const int FILTER_FLAG_QUERY_REQUIRED = 0;
const int FILTER_FLAG_IPV4 = 0;
const int FILTER_FLAG_IPV6 = 0;
const int FILTER_FLAG_NO_RES_RANGE = 0;
const int FILTER_FLAG_NO_PRIV_RANGE = 0;

<<__PHPStdLib, __Deprecated('will be removed in a future version of hack')>>
function filter_has_var(int $type, string $variable_name): bool {}
<<__PHPStdLib, __Rx>>
function filter_id(string $name) {}
<<__PHPStdLib, __Deprecated('will be removed in a future version of hack')>>
function filter_input_array(int $type, mixed $definition = null, bool $add_empty = true) {}
<<__PHPStdLib, __Deprecated('will be removed in a future version of hack')>>
function filter_input(int $type, string $variable_name, int $filter = FILTER_DEFAULT, mixed $options = null) {}
<<__PHPStdLib, __Rx>>
function filter_list(): array<string> {}
<<__PHPStdLib, __Rx>>
function filter_var_array(array $data, mixed $definition = null, bool $add_empty = true) {}
<<__PHPStdLib, __Rx>>
function filter_var(mixed $value, int $filter = FILTER_DEFAULT, mixed $options = null) {}

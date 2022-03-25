<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

namespace HH\Lib\Str;

use namespace HH\Lib\Locale;
use namespace HH\Lib\_Private\_Str;

/**
 * Given a valid format string (defined by `SprintfFormatString`), return a
 * formatted string using `$format_args`
 *
 * @guide /hack/built-in-types/string
 * @guide /hack/functions/format-strings
 */
function format_l(
  Locale\Locale $locale,
  SprintfFormatString $format_string,
  mixed ...$format_args
)[]: string {
  return _Str\vsprintf_l($locale, $format_string as string, $format_args);
}

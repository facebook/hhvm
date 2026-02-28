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
<<__NoAutoLikes>>
function format_l<Targs as (mixed...)>(
  Locale\Locale $locale,
  \HH\TypedFormatString<SprintfFormat, Targs> $format_string,
  ... Targs $format_args,
)[]: string {
  return _Str\vsprintf_l(
    $locale,
    $format_string,
    HH\FIXME\UNSAFE_CAST<(mixed...), vec<mixed>>($format_args),
  );
}

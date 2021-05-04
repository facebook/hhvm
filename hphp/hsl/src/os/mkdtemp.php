<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\OS;

use namespace HH\Lib\Str;
use namespace HH\Lib\_Private\_OS;

/** Create a temporary directory.
 *
 * This function creates a new, unique temporary directory, with the name
 * matching the provided template, and returns the path. The directory will be
 * created with permissions 0700.
 *
 * The template MUST end with `XXXXXX`; these are replaced with random
 * printable characters to create a unique name. While some platforms are more
 * flexible, the HSL always requires this for consistency. Any additional
 * trailing `X`s may result in literal X's (e.g. glibc), or in additional
 * randomness (e.g. BSD) - use a separator (e.g. `fooXXX.XXXXXX`) to guarantee
 * any characters are preserved.
 */
function mkdtemp(string $template): string {
  // This restriction exists with glibc, but BSD (e.g. MacOS) is more flexible;
  // don't let people accidentally write non-portable code.
  if (!Str\ends_with($template, 'XXXXXX')) {
    _OS\throw_errno(
      Errno::EINVAL,
      "mkdtemp template must always end with 'XXXXXX' (portability)",
    );
  }
  return _OS\wrap_impl(() ==> _OS\mkdtemp($template));
}

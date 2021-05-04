<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\File;

use namespace HH\Lib\{OS, Str};
use namespace HH\Lib\_Private\_File;

/** Create a new temporary file.
 *
 * The file is automatically deleted when the disposable is removed.
 *
 * - If the prefix starts with `.`, it is interpretered relative to the current
 *   working directory.
 * - If the prefix statis with `/`, it is treated as an absolute path.
 * - Otherwise, it is created in the system temporary directory.
 *
 * Regardless of the kind of prefix, the parent directory must exist.
 *
 * A suffix can optionally be provided; this is useful when you need a
 * particular filename extension; for example,
 * `File\temporary_file('foo', '.txt')` may create `/tmp/foo123456.txt`.
 *
 * The temporary file:
 * - will be a new file (i.e. `O_CREAT | O_EXCL`)
 * - be owned by the current user
 * - be created with mode 0600
 */
<<__ReturnDisposable>>
function temporary_file(
  string $prefix = 'hack-tmp-',
  string $suffix = '',
): TemporaryFile {
  if (
    !(
      Str\starts_with($prefix, '/') ||
      Str\starts_with($prefix, './') ||
      Str\starts_with($prefix, '../')
    )
  ) {
    /* HH_IGNORE_ERROR[2049] PHP stdlib */
    /* HH_IGNORE_ERROR[4107] PHP stdlib */
    $prefix = Str\trim_right(\sys_get_temp_dir(), '/').'/'.$prefix;
  }
  $pattern = $prefix.'XXXXXX'.$suffix;
  list($handle, $path) = OS\mkstemps($pattern, Str\length($suffix));
  return new TemporaryFile(new _File\CloseableReadWriteHandle($handle, $path));
}

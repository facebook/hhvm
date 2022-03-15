<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

namespace HH\Lib\_Private\_File;

use namespace HH\Lib\{OS, Str};

function open_temporary_fd(
  string $prefix,
  string $suffix,
): (OS\FileDescriptor, string) {
  if (
    !(
      Str\starts_with($prefix, '/') ||
      Str\starts_with($prefix, './') ||
      Str\starts_with($prefix, '../')
    )
  ) {
    $prefix = Str\trim_right(\sys_get_temp_dir(), '/').'/'.$prefix;
  }
  $pattern = $prefix.'XXXXXX'.$suffix;
  return OS\mkstemps($pattern, Str\length($suffix));
}

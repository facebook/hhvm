<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\_Private\_OS;
use namespace HH\Lib\{Dict, Keyset, OS};

<<__Memoize>>
function get_errno_names(): dict<OS\Errno, keyset<string>> {
  $values = OS\Errno::getValues();
  return Dict\group_by(
    Keyset\keys($values),
    $name ==> $values[$name],
  ) |> Dict\map($$, $names ==> keyset($names));
}

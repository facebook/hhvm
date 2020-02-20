<?hh // partial
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

$foo = function(array<string, mixed> $rel): array<string, mixed> {
  return darray[
    'id'       => $rel['id'],
    'relation' => $rel['relation_type'],
  ];
};

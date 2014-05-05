<?hh // decl
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

$foo = function(array<string, mixed> $rel): array<string, mixed> {
  return array(
    'id'       => $rel['id'],
    'relation' => $rel['relation_type'],
  );
};

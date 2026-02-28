<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

namespace NS;

function returns_dict(): dict<string, int> {
  return dict(dict['a' => 123, 'b' => 456, 'c' => 789]);
}

function returns_keyset(): keyset<string> {
  return keyset(vec['a', 'b', 'c']);
}

function returns_vec(): vec<string> {
  return vec(vec['a', 'b', 'c']);
}

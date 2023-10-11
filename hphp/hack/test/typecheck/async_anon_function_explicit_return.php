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

function foo(): Awaitable<int> {
  $x = async function() {
    return 5;
  };
  return $x();
}

function bar(): Awaitable<Awaitable<string>> {
  $p = async function() {
    $q = function() {
      $r = async function() {
        $s = function() {
          return "inception";
        };
        return $s();
      };
      return $r();
    };
    return $q();
  };
  return $p();
}

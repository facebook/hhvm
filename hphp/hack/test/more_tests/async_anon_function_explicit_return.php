 <?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
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

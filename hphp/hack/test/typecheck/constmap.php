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

function covariance(ConstMap<mixed, mixed> $x): void {}
function test1(Map<string, int> $c): ConstMap<mixed, mixed> {
  covariance($c);
  return $c;
}
function test2(ImmMap<string, int> $c): ConstMap<mixed, mixed> {
  covariance($c);
  return $c;
}

class C<Tv> {
  public function covariance(ConstMap<mixed, Tv> $x): void {}
  public function test1<Tu as Tv>(Map<string, Tu> $c): ConstMap<mixed, Tv> {
    $this->covariance($c);
    return $c;
  }
  public function test2<Tu as Tv>(ImmMap<string, Tu> $c): ConstMap<mixed, Tv> {
    $this->covariance($c);
    return $c;
  }
}

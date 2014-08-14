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

function covariance(ConstSet<mixed> $x): void {}
function test1(Set<int> $c): ConstSet<mixed> {
  covariance($c);
  return $c;
}
function test2(ImmSet<int> $c): ConstSet<mixed> {
  covariance($c);
  return $c;
}

class C<Tv> {
  public function covariance(ConstSet<Tv> $x): void {}
  public function test1<Tu as Tv>(Set<Tu> $c): ConstSet<Tv> {
    $this->covariance($c);
    return $c;
  }
  public function test2<Tu as Tv>(ImmSet<Tu> $c): ConstSet<Tv> {
    $this->covariance($c);
    return $c;
  }
}

<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

function covariance(ConstSet<arraykey> $x): void {}
function test1(Set<int> $c): ConstSet<arraykey> {
  covariance($c);
  return $c;
}
function test2(ImmSet<int> $c): ConstSet<arraykey> {
  covariance($c);
  return $c;
}

class C<Tv as arraykey> {
  public function covariance(ConstSet<Tv> $x): void {}
  public function test1<Tu as Tv>(Set<Tu> $c): ConstSet<Tv> {
    $this->covariance($c);
    return $c;
  }
  public function test2<Tu as Tv as arraykey>(ImmSet<Tu> $c): ConstSet<Tv> {
    $this->covariance($c);
    return $c;
  }
}

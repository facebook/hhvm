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

function covariance(ConstMap<arraykey, mixed> $x): void {}
function test1(Map<string, int> $c): ConstMap<arraykey, mixed> {
  covariance($c);
  return $c;
}
function test2(ImmMap<string, int> $c): ConstMap<arraykey, mixed> {
  covariance($c);
  return $c;
}

class C<Tv> {
  public function covariance(ConstMap<arraykey, Tv> $x): void {}
  public function test1<Tu as Tv>(Map<string, Tu> $c): ConstMap<arraykey, Tv> {
    $this->covariance($c);
    return $c;
  }
  public function test2<Tu as Tv>(
    ImmMap<string, Tu> $c
  ): ConstMap<arraykey, Tv> {
    $this->covariance($c);
    return $c;
  }
}

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

function covariance(ConstVector<mixed> $x): void {}
function test1(Vector<int> $c): ConstVector<mixed> {
  covariance($c);
  return $c;
}
function test2(ImmVector<int> $c): ConstVector<mixed> {
  covariance($c);
  return $c;
}

class C<Tv> {
  public function covariance(ConstVector<Tv> $x): void {}
  public function test1<Tu as Tv>(Vector<Tu> $c): ConstVector<Tv> {
    $this->covariance($c);
    return $c;
  }
  public function test2<Tu as Tv>(ImmVector<Tu> $c): ConstVector<Tv> {
    $this->covariance($c);
    return $c;
  }
}

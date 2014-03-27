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

namespace NS1\NS2 {
  trait MyTrait<T1, T2> {}

  function f(): void {
    f();
    \NS1\NS2\f();
  }

  class C<T> {
    use MyTrait<T, int>;

    public static function g(): void {
      C::g();
      \NS1\NS2\C::g();
    }

    public function h(): void {
      $this->h();
    }

    public function i(C<T> $x): \NS1\NS2\C<T> {
      return $x;
    }
  }

  function id<T>(T $x): T {
    return $x;
  }

  function id2<T as C>(T $x): T {
    return $x;
  }

  function id3<T as \NS1\NS2\C>(T $x): T {
    return $x;
  }
}

namespace NS3 {
  class C2 extends \NS1\NS2\C<int> {}
}

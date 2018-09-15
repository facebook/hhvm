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

class MyVector<T> {
  public static function fmap<T1, T2>(
    (function(T1): T2) $f,
    MyVector<T1> $x,
  ): MyVector<T2> {
    throw new Exception('test');
  }
}

function test(): void {
  $in = new MyVector();
  $in2 = MyVector::fmap<string, string>(
    function(int $x): string {
      return 'x';
    },
    $in,
  );
}

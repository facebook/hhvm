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

class MyVector<T> {
  public function add(T $x): this {
    return $this;
  }
  public function duck(MyVector<T> $v): void {}
}

class Foo {
  public function bar(): MyVector<Foo> {
    $ret = new MyVector();
    $ret->duck($ret);
    $node = $this;
    $ret->add($node);
    return $ret;
  }

  public function duck(): MyVector<Foo> {
    $ret = new MyVector();
    $node = $this;
    while ($node) {
      $ret->add($node);
    }
    return $ret;
  }

  public function getParent(): ?Foo {
    return null;
  }
}

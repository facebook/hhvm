<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */


class A {
  public function f(int $x): int { return 0; }
}

class B extends A {
  public function f(int $x, int $y = 0): int { return 0; }
  public function g(int $x): int { return 0; }

}

class MyList<T> {
  private Vector<T> $l;

  public function __construct(Vector<T> $l) {
    $this->l = $l;
  }

  public function get_head(): T {
    return $this->l[0];
  }

  public function get_list(): Vector<T> {
    return $this->l;
  }
}

class MyNewList extends MyList<int> {

  public function get_add1(): int {
    return $this->get_head();
  }
}

class MyDoubleList<T1,T2> extends MyList<(T1,T2)> {

  public function get_head_twice(): (T1,T2) {
    return $this->get_head();
  }

  public function set_head(T1 $x, T2 $y): void {
    $this->l[] = tuple($x, $y);
  }
}


$x = new MyNewList();
dump_var($x->get_head());

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

class Gen<T> {
  public function __construct(public T $data) {}
}

<<__ConsistentConstruct>>
class Base<T> {
  public function __construct(private Gen<T> $data) {}
  public function get(): T {
    return $this->data->data;
  }
  public function set($val): this {
    return $this;
  }
  public function __call(string $name, array $args): this {
    return $this;
  }

  public static function make(Gen<T> $data): this {
    return new static($data);
  }
}

class X {}
class Child extends Base<X> {}

final class Another<T> {
  private ?Gen<T> $data;
  public function set(Gen<T> $data): this {
    $this->data = $data;
    return $this;
  }
  public function __call(string $name, array $args): ?T {
    return $this->data ? $this->data->data : null;
  }
  public static function make(Gen<T> $data): this {
    $i = new static();
    $i->set($data);
    return $i;
  }
}

function takesX(X $arg): void {}

function test(int $i): Base {
  $x = new X();
  switch ($i) {
    case 0:
      $thing = new Base(new Gen($x));
      break;
    case 1:
      $thing = Base::make(new Gen($x));
      break;
    case 2:
      $thing = new Child(new Gen($x));
      break;
    default:
      $thing = Child::make(new Gen($x));
      break;
  }
  $chain = $thing->set(1)->set('foo')->set(new stdClass());
  $after_chain = $chain->get();
  // hh_show($after_chain);
  takesX($after_chain);

  $ret = $chain->magic()->magic(1, 2)->magic(1,2,'foo',4,new stdClass());
  $dt = $ret->get();
  // hh_show($dt);
  takesX($dt);

  return $ret;
}

function test_generic_non_this(): void {
  $thing = new Another();
  // hh_show($thing);
  $s = $thing->set(new Gen(new X()));
  // hh_show($s);
  $x = $s->magicGetter();
  // hh_show($x);
  if (null !== $x) {
    takesX($x);
  }
}

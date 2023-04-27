<?hh // strict

class Bar {
  public int $prop = 0;
}

class Foo {
  public static int $static_prop = 1;
  <<__LateInit>> public static Bar $bar;
  public int $instance_prop = 0;
  <<__LateInit>> public Bar $instance_bar;

  <<__Memoize>>
  public static function foo_method_memoized(): int {
    return self::$static_prop;
  }

  <<__MemoizeLSB>>
  public static function foo_method_memoized_lsb(): Bar {
    return self::$bar; // A global variable is passed to (or returned from) a function call.
  }

  <<__Memoize>>
  public function foo_instance_method_memoized(): int {
    return $this->instance_prop;
  }

  <<__Memoize>>
  public function foo_instance_method_memoized_lsb(): Bar {
    return $this->instance_bar;
  }
}

<<__Memoize>>
function fun_memoized(): Bar {
  return new Bar();
}

<<__Memoize>>
function fun_memoized_vec_int(): vec<int> {
  return vec[1, 2, 3];
}

class Test {
  public function test_method_call(): void {
    (Foo::$bar)->prop = 1; // A global variable is written.

    $a = Foo::foo_method_memoized();
    $a = 2;

    $b = Foo::foo_method_memoized_lsb();
    $b->prop = 2; // A global variable is written.

    $c = fun_memoized();
    $c->prop = 2; // A global variable is written.

    $d = fun_memoized_vec_int();
    $d[0] = 2;

    $e = new Foo();
    $f = $e->foo_instance_method_memoized();
    $f = 2;

    $g = $e->foo_instance_method_memoized_lsb();
    $g->prop = 2;

    $h = true ? new Foo() : null;
    $i = $h?->foo_instance_method_memoized_lsb();
    if ($i is nonnull) {
      $i->prop = 2;
    }
  }
}

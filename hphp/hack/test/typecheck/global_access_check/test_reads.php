<?hh // strict

class Bar {
  public int $prop = 0;
}

class Foo {
  public static int $static_prop = 1;
  <<__LateInit>> public static Bar $bar;
}

function call_mixed(mixed $x): void {}

<<__Memoize>>
function fun_memoized(int $i, string $s): Bar {
  return new Bar();
}

function test_fun_call(): int {
  $a = Foo::$bar; // global read
  $a->prop = 2;
  return (Foo::$bar)->prop + 1; // global read
}

class Test {
  private static int $static_prop = 0;

  public function test_method_call(): int {
    self::$static_prop++; // global read?

    if (self::$static_prop > 0) { // global read
      (Foo::$bar)->prop = self::$static_prop + 1; // global read
    }

    (Foo::$bar)->prop = self::$static_prop > 0 ? self::$static_prop + 1 : 0; // 2 global reads

    call_mixed((Foo::$bar)->prop); // global read
    call_mixed(self::$static_prop); // global read

    $x = new Bar();
    $a = fun_memoized(1, "a"); // global read
    $b = fun_memoized(1, "a")->prop + 1; // global read
    $c = fun_memoized(1, "a")->prop > 0 ? 1 : -1; // global read

    if (!(fun_memoized(1, "a")->prop > 0)) { // global read
      fun_memoized(1, "a")->prop = 1;
    }

    return self::$static_prop; // global read
  }

  public function test_loop(): void {
    $a = new Bar();
    $b = new Bar();
    $c = new Bar();
    for ($i = 0; $i < Foo::$static_prop; $i++) { // global read
      $a = Foo::$bar; // global read
      $c = $b;
      $b = $a;
    }
  }
}

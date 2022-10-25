<?hh // strict

class Bar {
  public int $prop = 0;
}

class Foo {
  public static int $static_prop = 1;
  <<__LateInit>> public static Bar $bar;
}

function call_mixed(mixed $x): void {}

function test_fun_call(): Bar {
  $a = Foo::$bar;
  $a->prop = 2; // A global variable is written.
  return $a; // A global variable is passed to (or returned from) a function call.
}

class Test {
  private static int $static_prop = 0;

  public function test_method_call(): void {
    self::$static_prop = 1; // A global variable is written.
    /* HH_FIXME[11001] Test HH_FIXME to ignore the error of static variable direct write */
    self::$static_prop = 1;

    Foo::$static_prop = 2; // A global variable is written.
    Foo::$static_prop++; // A global variable is written.

    (Foo::$bar)->prop = 2; // A global variable is written.
    Foo::$bar = new Bar(); // A global variable is written.

    $a = Foo::$static_prop;
    $a = 2;
    $a++;

    $b = Foo::$bar;
    $b->prop = 2; // A global variable is written.
    /* HH_FIXME[11002] Test HH_FIXME to ignore the error of global variable write */
    $b->prop = 2; // A global variable is written.
    $b->prop++; // A global variable is written.

    $c = $b;
    call_mixed($c); // A global variable is passed to (or returned from) a function call.
    /* HH_FIXME[11003] Test HH_FIXME to ignore the error of passing global variables to function calls */
    call_mixed($c);
    call_mixed(Foo::$static_prop);

    $d = $c->prop;
    $d = 3;

    $b = new Bar();
    $b->prop = 2;
  }
}

<?hh

class Bar {
  public int $prop = 0;
}

class Foo {
  public static int $static_prop = 1;
  <<__SafeForGlobalAccessCheck>> public static int $static_prop_ignored = 1;
  <<__LateInit>> public static Bar $bar;
  <<__LateInit, __SafeForGlobalAccessCheck>> public static Bar $bar_ignored;
}

function call_mixed(mixed $x): void {}

class Test {
  private static int $static_prop = 0;
  <<__SafeForGlobalAccessCheck>> private static int $static_prop_ignored = 0;

  public function test_method_call(): void {
    self::$static_prop = 1; // A global variable is written.
    /* HH_FIXME[11001] Test HH_FIXME to ignore the error of static variable direct write */
    self::$static_prop = 1;
    self::$static_prop_ignored = 1; // Ignored: a global variable is written.

    Foo::$static_prop = 2; // A global variable is written.
    /* HH_FIXME[11001] Test HH_FIXME to ignore the error of static variable direct write */
    Foo::$static_prop = 2;
    Foo::$static_prop_ignored = 2; // Ignored: a global variable is written.

    (Foo::$bar)->prop = 2; // A global variable is written.
    (Foo::$bar_ignored)->prop = 2; // Ignored: a global variable is written.
    Foo::$bar = new Bar(); // A global variable is written.
    Foo::$bar_ignored = new Bar(); // Ignored: a global variable is written.

    $a = Foo::$bar;
    $a->prop = 2; // A global variable is written.
    /* HH_FIXME[11002] Test HH_FIXME to ignore the error of global variable write */
    $a->prop = 2; // A global variable is written.

    $b = Foo::$bar_ignored;
    $b->prop = 2; // Ignored: a global variable is written.

    $c = $a;
    call_mixed($c); // A global variable is passed to (or returned from) a function call.
    /* HH_FIXME[11003] Test HH_FIXME to ignore the error of passing global variables to function calls */
    call_mixed($c);
  }
}

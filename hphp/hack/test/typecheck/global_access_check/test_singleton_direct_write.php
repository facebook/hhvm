<?hh // strict

class Bar {
  public int $prop = 0;
}

class Foo {
  public static ?Bar $bar = null;
}

/* This is a test of recognizing the sigleton pattern for global writes.
 * Here in some conditionals, we directly write to Foo::$bar when its value is null.
 * Notice that in this example, Foo::$bar cannot be indirectly written via reference,
 * e.g. "$a = Foo::$bar; if ($a is null) { $a = new Bar(); }" does not really mutate
 * the static variable Foo::$bar.
 * Yet, in another test "test_singleton_write_via_ref.php", indirect writing to global
 * variables via reference is tested separately.
 */
class Test {
  public function test_method_call(): void {
    if (Foo::$bar is null) {
      Foo::$bar = new Bar(); // Singleton
    }

    Foo::$bar = new Bar();

    if (Foo::$bar is null) {
      Foo::$bar = new Bar(); // Singleton
    } else {
      (Foo::$bar)->prop = 1;
    }

    $a = Foo::$bar;
    if ($a is nonnull) {
      $a->prop = 1;
    } else {
      Foo::$bar = new Bar(); // Singleton
    }

    if (Foo::$bar === null) {
      Foo::$bar = new Bar(); // Singleton
    } else {
      (Foo::$bar)->prop = 1;
    }

    if ($a !== null) {
      $a->prop = 1;
    } else {
      Foo::$bar = new Bar(); // Singleton
    }

    if (Foo::$bar == null) {
      Foo::$bar = new Bar(); // Singleton
    } else {
      (Foo::$bar)->prop = 1;
    }

    if ($a != null) {
      $a->prop = 1;
    } else {
      Foo::$bar = new Bar(); // Singleton
    }
  }
}

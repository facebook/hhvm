<?hh // strict

class Baz {
  public int $prop = 0;
}

class Bar {
  public ?Baz $baz = null;
}

class Foo {
  public static ?Bar $bar = null;
}

<<__Memoize>>
function fun_memoized(): Bar {
  return new Bar();
}

/* This is a test of recognizing the sigleton pattern for global writes.
 * Here in some conditionals, we write to the property $baz of Foo::$bar via reference,
 * when the value of $baz is null.
 * As the patterns of format "=== null", "!== null", "== null" and "!= null" are already
 * tested in "test_singleton_direct_write.php", they are omitted here.
 */
class Test {
  public function test_method_call(): void {
    if (Foo::$bar is null) {
      Foo::$bar = new Bar(); // Singleton
    }

    $a = Foo::$bar;
    if ($a->baz is null) {
      $a->baz = new Baz(); // Singleton
    } else {
      ($a->baz)->prop = 1;
    }

    if ($a->baz is nonnull) {
      ($a->baz)->prop = 1;
    } else {
      $a->baz = new Baz(); // Singleton
    }

    $b = fun_memoized();
    if ($b->baz is null) {
      $b->baz = new Baz(); // Singleton
    } else {
      ($b->baz)->prop = 1;
    }

    if ($b->baz is nonnull) {
      ($b->baz)->prop = 1;
    } else {
      $b->baz = new Baz(); // Singleton
    }
  }
}

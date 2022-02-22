<?hh // strict

class Bar {
  public int $prop = 0;
}

class Foo {
  <<__LateInit>> public static Bar $bar;
}

class Test {
  public function test_method_call(): void {
    (Foo::$bar)->prop = 1; // A global variable is written.

    $x = new Bar();
    if (false) {$x = Foo::$bar;}
    $x->prop = 1; // A global variable is written.

    $y = Foo::$bar;
    if (false) {$y = new Bar();}
    $y->prop = 1; // A global variable is written.

    $z = new Bar();
    if (false) {
      $z = Foo::$bar;
      $z->prop = 1; // A global variable is written.
    } else {
      $z = new Bar();
      $z->prop = 2;
    }
    $z->prop = 3; // A global variable is written.
  }
}

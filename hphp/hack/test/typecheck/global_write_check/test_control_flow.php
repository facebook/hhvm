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

    $a = Foo::$bar;
    $b = new Bar();
    $c = new Bar();
    for ($i = 0; $i < 3; $i++) {
      $a->prop = $i; // A global variable is written. (For 1+ iterations)
      $b->prop = $i; // A global variable is written. (For 2+ iterations)
      $c->prop = $i; // A global variable is written. (For 3+ iterations)
      $c = $b;
      $b = $a;
    }
    $a->prop = 1; // A global variable is written.
    $b->prop = 1; // A global variable is written. (If the loop has 1+ iterations)
    $c->prop = 1; // A global variable is written. (If the loop has 2+ iterations)

    $d = Foo::$bar;
    $e = new Bar();
    $j = 0;
    while ($j < 3) {
      $tmp = $d;
      $d = $e;
      $e = $tmp;
      $j++;
    }
    // After an even number of interations, $d is global while $e is not
    // After an odd number of interations, $e is global while $d is not
    $d->prop = 1; // A global variable is written.
    $e->prop = 1; // A global variable is written.
  }
}

<?hh // strict

class Bar {
  public int $prop = 0;

  public function get_prop(): int {
    return $this->prop;
  }

  public function set_prop(int $i): void {
    $this->prop = $i;
  }
}

class Baz {
  <<__LateInit>> public Bar $bar1;
  <<__LateInit>> public Bar $bar2;
}

class Foo {
  <<__LateInit>> public static Bar $bar;
}

class Test {
  public function test_method_call(): void {
    Foo::$bar = new Bar(); // A global variable is written.
    (Foo::$bar)->prop = 1; // A global variable is written.

    $a = Foo::$bar;
    $a->prop = 1; // A global variable is written.
    $a->set_prop(1); // To do: raise an error "A global variable is written."

    $b = $a->prop;
    $b = 2;

    $a = new Bar();
    $a->prop = 2;

    $c = new Baz();
    $c->bar1 = new Bar();
    $c->bar1->prop = 1;

    $c->bar2 = Foo::$bar;
    $c->bar2->prop = 1; // A global variable is written.
    $c->bar1->prop = 1; // A global variable is written. To do: remove this error.

    $c->bar1 = new Bar(); // A global variable is written. To do: remove this error.
    $c->bar2->prop = 1; // A global variable is written.
    $c->bar1->prop = 1; // A global variable is written. To do: remove this error.
  }
}

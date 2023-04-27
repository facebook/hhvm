<?hh // strict

class Bar {
  public int $prop = 0;
}

class Foo {
  public static int $static_prop = 1;
  <<__LateInit>> public static Bar $bar;
}

class Test {
  public function test_method_call(): void {
    $a = Foo::$bar;
    $fun = (int $val): Bar ==> {
      Foo::$static_prop = 2; // A global variable is written.
      $a->prop = $val; // A global variable is written.

      $a = new Bar();
      $a->prop = $val;

      return $a;
    };
    $a->prop = 2; // A global variable is written.

    $b = $fun(3);
    $b->prop = 4;
  }
}

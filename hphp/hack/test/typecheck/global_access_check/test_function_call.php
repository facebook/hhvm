<?hh

class Bar {
  public int $prop = 0;
}

class Foo {
  <<__LateInit>> public static Bar $bar;
}

function plus_one(Bar $b): void {
  $b->prop++; // A global variable is written
}

class Test {
  public function plus_two(Bar $b): void {
    $b->prop += 2; // A global variable is written
  }

  public function test_method_call(): void {
    Foo::$bar = new Bar(); // A global variable is written

    // Test function call
    plus_one(Foo::$bar); // A global variable is passed to a function call

    // Test anonymous function call
    $fun = (Bar $b): void ==> {
      $b->prop = 2; // A global variable is written
    };
    $fun(Foo::$bar); // A global variable is passed to an anonymous function

    // Test method call
    $this->plus_two(Foo::$bar); // A global variable is passed to a method
  }
}

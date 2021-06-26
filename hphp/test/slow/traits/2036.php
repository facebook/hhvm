<?hh

trait TestTrait {
  public static function test() {
    $fun = varray['A', 'test'];
    return 'Forwarded '.$fun();
  }
}

class A {
  <<__DynamicallyCallable>> public static function test() {
    return "Test A";
  }
}

class B extends A {
  use TestTrait;
}
<<__EntryPoint>> function main(): void {
  echo B::test();
}

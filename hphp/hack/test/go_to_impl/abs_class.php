<?hh
abstract class Foo {

  public function test(): string {
    return "a";
  }
}

class Bar extends Foo {
  function test(): string {
    return "b";
  }
}

class Baz extends Bar {
  function test(): string {
    return "c";
  }
}

class Random {
  function test(): string {
    return "d";
  }
}

<?hh
class Foo {

  public function test(): string {
    return "a";
  }
}

class Bar extends Foo {
  function test(): string {
    return "b";
  }
}

class Baz {
  function test(Foo $f): string {
    return $f->test();
  }
}

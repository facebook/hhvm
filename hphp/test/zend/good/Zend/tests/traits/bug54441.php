<?hh

trait Foo {
  public function bar() {}
}

class Boo {
  use Foo {
    bar as dontKnow;
    dontKnow as protected;
  }
}

<<__EntryPoint>> function main(): void {}

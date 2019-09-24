<?hh

trait baz  {
  public function bar() {
    yield 1;
  }
}

class foo {
  use baz;
  public function bar() {
  }
}

<<__EntryPoint>> function main(): void { echo "Done.\n"; }

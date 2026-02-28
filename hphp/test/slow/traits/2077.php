<?hh

trait baz  {
  public function bar() :AsyncGenerator<mixed,mixed,void>{
    yield 1;
  }
}

class foo {
  use baz;
  public function bar() :mixed{
  }
}

<<__EntryPoint>> function main(): void { echo "Done.\n"; }

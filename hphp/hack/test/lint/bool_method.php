<?hh

class Foo {
  public function __bool(): void {}
}

// A second definition to silence the lint about the name of files
// with a single definition.
function bar(): void {}

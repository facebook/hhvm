<?hh // strict

class Bar implements IMemoizeParam {
  public function getInstanceKey(): string {
    return "dummy";
  }
}

class Foo {
  <<__Memoize>>
  public function someMethod((int, Bar, string) $tup): void {}
}

<<__Memoize>>
function some_function((int, Bar, string) $tup): void {}

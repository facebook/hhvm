<?hh

class Bar implements IMemoizeParam {
  public function getInstanceKey(): string {
    return "dummy";
  }
}

class Foo {
  <<__Memoize>>
  public function someMethod(int $i, Bar $arg, string $str): string {
    return "hello";
  }
}

<<__Memoize>>
function some_function(int $i, Bar $arg, string $str): string {
  return 'goodbye';
}

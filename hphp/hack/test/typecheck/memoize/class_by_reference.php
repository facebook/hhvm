<?hh

class Bar implements IMemoizeParam {
  public function getInstanceKey(): string {
    return "dummy";
  }
}

class Foo {
  <<__Memoize>>
  public function someMethod(Bar &$arg): string {
    return "hello";
  }
}

<<__Memoize>>
function some_function(Bar &$arg): string {
  return 'hello';
}

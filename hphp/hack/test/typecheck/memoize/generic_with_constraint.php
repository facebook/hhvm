<?hh

class Bar implements IMemoizeParam {
  public function getInstanceKey(): string {
    return "dummy";
  }
}

class Foo<T as Bar> {
  <<__Memoize>>
  public function someMethod(T $arg): string {
    return "hello";
  }
}

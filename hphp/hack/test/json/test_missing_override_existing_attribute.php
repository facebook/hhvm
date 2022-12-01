<?hh

class MyParent {
  <<__Memoize>>
  public function foo(): void {}
}

class MyChild extends MyParent {
  <<__Memoize>>
  public function foo(): void {}
}

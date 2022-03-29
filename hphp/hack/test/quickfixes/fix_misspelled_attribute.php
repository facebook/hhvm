<?hh

class MyParent {
  public function foo(): void {}
}

class MyChild extends MyParent {
  <<__Overide>>
  public function foo(): void {}
}

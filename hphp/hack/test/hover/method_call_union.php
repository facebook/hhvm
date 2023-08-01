<?hh

class FooParent {
  public function bar(): string { return "A"; }
}
class Other {
  public function bar(): int { return 3; }
}

class Foo extends FooParent {
  public function callIt(Other $obj, bool $b): void {
    if ($b) $obj = $this;
    $obj->bar();
    //    ^ hover-at-caret
  }
}

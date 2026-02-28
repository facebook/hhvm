<?hh
class A {
  private function foo(): void {}
}

class B extends A {
  // is fine, private function isn't inherited
  public function FOO(): void {}
}

<?hh

class ClassWithFooBar {
  public function foobar(): void {}
}

function call_method(ClassWithFooBar $mc): void {
  $mc->foobar();
}

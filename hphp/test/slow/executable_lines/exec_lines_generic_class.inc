<?hh

class OtherClass<T> {
  public function foo(): ?T {
    return null;
  }
}

final class SomeClass extends OtherClass<string> {
  public function bar(): ?string {
    return $this->foo();
  }
}

<?hh

class AGenericClass<T> {
  public function foo(): ?T {
    return null;
  }
}

final class AClassThatInheritsFromGeneric extends AGenericClass<string> {
  public function bar(): ?string {
    return $this->foo();
  }
}

function cover_generic_class() {
  $c = new AClassThatInheritsFromGeneric();
  $c->foo();
  $c->bar();
}

<?hh

module A;

class A {
  protected internal function foo(): void {
    echo 'in foo';
  }
}

class B extends A {
  private function foo(): void {
    echo 'in overridden foo';
  }

  public function foobar(): void {
    $this->foo();
  }
}

<<__EntryPoint>>
function main() {
  (new B())->foobar();
}

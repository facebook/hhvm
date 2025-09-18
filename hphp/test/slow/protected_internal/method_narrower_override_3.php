<?hh

module A;

class A {
  internal function foo(): void {
    echo 'in foo';
  }
}

class B extends A {
  protected internal function foo(): void {
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

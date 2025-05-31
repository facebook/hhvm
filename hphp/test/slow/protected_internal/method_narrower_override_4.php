<?hh
<<file: __EnableUnstableFeatures("protected_internal")>>

module A;

class A {
  protected function foo(): void {
    echo 'in foo';
  }
}

class B extends A {
  protected internal function foo(): void {
    // OK since at runtime protected internal treated as protected
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

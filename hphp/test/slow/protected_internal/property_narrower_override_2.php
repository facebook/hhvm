<?hh
<<file: __EnableUnstableFeatures("protected_internal")>>

module A;

class A {
  public $foo = 'foo';
}

class B extends A {
  protected internal $foo = 'overridden foo';

  public function foobar(): void {
    echo $this->foo;
  }
}

<<__EntryPoint>>
function main() {
  (new B())->foobar();
}

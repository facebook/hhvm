<?hh

module A;

class A {
  protected $foo = 'foo';
}

class B extends A {
  // OK since at runtime protected internal treated as protected
  protected internal $foo = 'overridden foo';

  public function foobar(): void {
    echo $this->foo;
  }
}

<<__EntryPoint>>
function main() {
  (new B())->foobar();
}

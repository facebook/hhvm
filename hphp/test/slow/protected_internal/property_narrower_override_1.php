<?hh

module A;

class A {
  protected internal $foo = 'foo';
}

class B extends A {
  private $foo = 'overridden foo';

  public function foobar(): void {
    echo $this->foo;
  }
}

<<__EntryPoint>>
function main() {
  (new B())->foobar();
}

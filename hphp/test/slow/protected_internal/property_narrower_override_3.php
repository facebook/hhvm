<?hh

module A;

class A {
  internal $foo = 'foo';
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

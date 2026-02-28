<?hh

module A;

class A {
  protected internal $foo = 'foo';

  protected internal static $static_foo = 'static foo';
}

class B extends A {
  protected internal $foo = 'overridden foo';

  protected internal static $static_foo = 'overridden static foo';

  public function foobar(): void {
    echo $this->foo."\n";
    echo static::$static_foo;
  }
}

<<__EntryPoint>>
function main() {
  (new B())->foobar();
}

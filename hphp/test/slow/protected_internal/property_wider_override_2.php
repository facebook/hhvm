<?hh
<<file: __EnableUnstableFeatures("protected_internal")>>

module A;

class A {
  protected internal $foo = 'foo';

  protected internal static $static_foo = 'static foo';
}

class B extends A {
  protected $foo = 'overridden foo';

  protected static $static_foo = 'overridden static foo';

  public function foobar(): void {
    echo $this->foo."\n";
    echo static::$static_foo;
  }
}

<<__EntryPoint>>
function main() {
  (new B())->foobar();
}

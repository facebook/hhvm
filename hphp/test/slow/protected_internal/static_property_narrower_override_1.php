<?hh
<<file: __EnableUnstableFeatures("protected_internal")>>

module A;

class A {
  protected internal static $foo = 'foo';
}

class B extends A {
  private static $foo = 'overridden foo';

  public function foobar(): void {
    echo static::$foo;
  }
}

<<__EntryPoint>>
function main() {
  (new B())->foobar();
}

<?hh

module A;

class A {
  protected static $foo = 'foo';
}

class B extends A {
  // OK since at runtime protected internal treated as protected
  protected internal static $foo = 'overridden foo';

  public function foobar(): void {
    echo static::$foo;
  }
}

<<__EntryPoint>>
function main() {
  (new B())->foobar();
}

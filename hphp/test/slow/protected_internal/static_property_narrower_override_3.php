<?hh

module A;

class A {
  internal static $foo = 'foo';
}

class B extends A {
  protected internal static $foo = 'overridden foo';

  public function foobar(): void {
    echo static::$foo;
  }
}

<<__EntryPoint>>
function main() {
  (new B())->foobar();
}

<?hh

module A;

class A {
  protected internal function foo(): void {
    echo "in foo\n";
  }

   protected internal static function static_foo(): void {
    echo "in static foo\n";
  }
}

class B extends A {
  internal function foo(): void {
    echo "in overridden foo\n";
  }

  internal static function static_foo(): void {
    echo "in overridden static foo\n";
  }

  public function foobar(): void {
    $this->foo();
    static::static_foo();
  }
}

<<__EntryPoint>>
function main() {
  (new B())->foobar();
}

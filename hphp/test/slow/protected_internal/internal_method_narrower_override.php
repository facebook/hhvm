<?hh
<<file: __EnableUnstableFeatures("protected_internal")>>

module A;

class A {
  internal function foo(): void {
    echo "in foo\n";
  }

  internal static function static_foo(): void {
    echo "in static foo\n";
  }
}

class B extends A {
  public function foo(): void {
   // OK since at runtime internal treated as public
    echo "in overridden foo\n";
  }

  public static function static_foo(): void {
    // OK since at runtime internal treated as public
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

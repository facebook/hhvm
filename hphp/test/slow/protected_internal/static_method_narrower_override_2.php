<?hh
<<file: __EnableUnstableFeatures("protected_internal")>>

module A;

class A {
  public static function foo(): void {
    echo 'in foo';
  }
}

class B extends A {
  protected internal static function foo(): void {
    echo 'in overridden foo';
  }

  public function foobar(): void {
    static::foo();
  }
}

<<__EntryPoint>>
function main() {
  (new B())->foobar();
}

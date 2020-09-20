<?hh

// We do not allow the creation of function pointers to instance methods
// from self, static, parent keywords (or any instance methods in general)

class A {
  public function foo(): void {
    echo "Hello";
  }
}

final class B extends A {
  <<__Override>>
  public function foo(): void {
    parent::foo<>;
    self::foo<>;
    static::foo<>;
  }
}

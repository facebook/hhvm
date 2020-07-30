<?hh

class A {
  public function foo(): void {
    echo "Hello";
  }
}

class B extends A {
  <<__Override>>
  public function foo(): void {
    parent::foo<>;
    self::foo<>;
    static::foo<>;
  }
}

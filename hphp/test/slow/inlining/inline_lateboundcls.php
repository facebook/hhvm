<?

class A {
  public static function foo() {
    return static::$bar;
  }
}

class B extends A {
  public static $bar = 42;
}

class C extends A {
  public static $bar = 43;
}

function main() {
  return B::foo() + C::foo();
}

echo var_dump(main());

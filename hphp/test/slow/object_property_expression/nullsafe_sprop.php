<?hh

class A1 {
  public int $foo = 42;
}

class A2 {
  public static ?A1 $bar;

  public function __construct() {
    self::$bar = new A1();
  }

  public function baz(): ?int {
    return self::$bar?->foo;
  }
}

echo (new A2())->baz()."\n";

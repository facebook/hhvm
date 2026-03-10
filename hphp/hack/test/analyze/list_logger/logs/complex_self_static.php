<?hh

class Base {
  public static int $p = 0;
}

class Baz extends Base {
  public static int $x = 0;
  public static string $y = "";
  public int $a = 0;

  public function test_self(): void {
    list(self::$x, $b) = tuple(1, 2);
  }

  public function test_static(): void {
    list(static::$y, $c) = tuple("hi", 42);
  }

  public function test_this(): void {
    list($this->a, $d) = tuple(1, 2);
  }

  public function test_parent(): void {
    list(parent::$p, $e) = tuple(3, 4);
  }
}

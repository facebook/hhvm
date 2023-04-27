<?hh // strict

class Foo {
  public static int $static_int = 0;
  public static string $static_string = "";

  public static function get_int(): int {
    return self::$static_int;
  }

  public static function set_int(int $i): void {
    self::$static_int = $i; // A global variable is written.
  }

  public static function get_string(): string {
    return self::$static_string;
  }

  public static function set_string(string $s): void {
    self::$static_string = $s; // A global variable is written.
  }
}


class Test {
  public static ?int $static_int = null;
  public static ?string $static_string = null;

  public function test_method_call(): void {
    Foo::$static_int = 1; // A global variable is written.
    Foo::$static_string = "Foo"; // A global variable is written.

    $a = Foo::$static_int;
    $a = 2;

    $b = Foo::$static_string;
    $b = "Foo2";

    Foo::set_int(3);
    Foo::set_string("Foo3");

    self::$static_int = Foo::get_int(); // A global variable is written.
    self::$static_string = Foo::get_string(); // A global variable is written.
  }
}

<?hh

interface C {
  public static function foo(): void;
}

trait D {
  require implements C;

  public function test(): void {
    $x = self::foo<>;
  }
}

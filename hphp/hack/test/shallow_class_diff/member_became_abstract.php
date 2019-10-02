<?hh

abstract class C {
  const type T = int;
  public ?int $x;
  public static ?int $y;
  public function f(): void {}
  public static function g(): void {}
  public function __construct() {}
}

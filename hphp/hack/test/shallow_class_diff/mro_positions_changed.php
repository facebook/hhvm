<?hh

class C implements I {
  use T;

  const int X = 0;
  const type T = int;

  private ?int $x;
  private static ?int $y;
  private function f(): void {}
  private static function g(): void {}
}

<?hh

abstract class Foo {
  const int BAR = 1;

  public function test(): void {
    $_ = (int $x = static::BAR): void ==> {};
    $_ = function(string $y = static::class): void {};
  }
}

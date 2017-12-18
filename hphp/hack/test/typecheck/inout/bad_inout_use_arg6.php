<?hh // strict

function f(inout int $i): void {}

class C {
  protected static int $x = 42;

  public function test(): void {
    f(inout static::$x);
  }
}

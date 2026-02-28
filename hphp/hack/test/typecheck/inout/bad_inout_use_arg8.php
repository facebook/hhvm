<?hh

function f(inout int $i): void {}

class C {
  public static int $x = 42;
}

function test(): void {
  $c = new C();
  f(inout $c::$x);
}

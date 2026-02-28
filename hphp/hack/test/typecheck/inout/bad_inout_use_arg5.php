<?hh

function f(inout int $i): void {}

class C {
  public int $x = 42;
}

function test(): void {
  $v = vec[new C()];
  f(inout $v[0]->x);
}

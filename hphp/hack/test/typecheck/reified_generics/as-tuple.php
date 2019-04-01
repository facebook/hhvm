<?hh

class C<<<__Enforceable>> reify T1> {
  public function f<<<__Enforceable>> reify T2>(mixed $x): void {
    $x as (T1, T2);
  }
}

function fc(): void {
  $c = new C<string>();
  $c->f<int>(tuple("hello", 1));
  $c->f<int>(tuple(1, "hello"));
}

function g<reify T1, reify T2>(): void {
  f<(int, int), int>(1, 2);
}

function f<reify T1, reify T2>(int $x, int $y): void {
  g<(int, (T1, T2)), T1>();
}

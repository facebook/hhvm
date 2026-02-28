<?hh

function f(mixed $m): void {
  $m;
}

function g(mixed $m): void {
  $m;
}

class C {
  public function f(mixed $m): void {
    $m;
  }

  public function g(mixed $m): void {
    $m;
  }
}

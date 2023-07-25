<?hh
function f(int $x): void {
  match ($x) {
    _ -> { print('x'); }
  }
}

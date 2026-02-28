<?hh
function f(int $x): void {
  match ($x) {
    case _ => { print('x'); }
  }
}

<?hh

function f(mixed $v): void {
  match ($v) {
    $i: int => { echo "int $i\n"; }
    _: string => { echo "string $v\n"; }
    _ => { echo "unknown\n"; }
    $x => { echo "unreachable\n"; }
  }
}

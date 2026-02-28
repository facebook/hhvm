<?hh

<<file:__EnableUnstableFeatures('match_statements')>>

function f(arraykey $v): void {
  match ($v) {
    $i: int => echo "int $i\n";
    _ => echo "string $v\n";
  }
}

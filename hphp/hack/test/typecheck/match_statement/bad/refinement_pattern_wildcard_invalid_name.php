<?hh

<<file:__EnableUnstableFeatures('match_statements')>>

function f(arraykey $v): void {
  match ($v) {
    unused: int => echo "int $v\n";
    _ => echo "string $v\n";
  }
}

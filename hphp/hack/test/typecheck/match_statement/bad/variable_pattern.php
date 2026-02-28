<?hh

<<file:__EnableUnstableFeatures('match_statements')>>

function f(int $v): void {
  match ($v) {
    $i => echo "int $i\n";
  }
}

<?hh

<<file:__EnableUnstableFeatures('match_statements')>>

function f(arraykey $v): void {
  match ($v) {
    unused => echo "arraykey $v\n";
  }
}

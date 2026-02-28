<?hh

<<file:__EnableUnstableFeatures('match_statements')>>

function f(Option<int> $o): void {
  match ($o) {
    Some(_) => echo "Some\n";
    None => echo "None\n";
  }
}

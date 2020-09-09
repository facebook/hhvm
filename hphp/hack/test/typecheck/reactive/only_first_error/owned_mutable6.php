<?hh // partial

function f() {
  // ERROR
  $x = (<<__OwnedMutable>> A $c) ==> {};
}

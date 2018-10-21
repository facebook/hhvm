<?hh

function f() {
  // ERROR
  $x = (<<__OwnedMutable>> A $c) ==> {};
}

<?hh

function f(): void {
  // ERROR
  $x = (<<__OwnedMutable>> A $c) ==> {};
}

interface A {}

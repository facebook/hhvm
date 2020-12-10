<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

function f(): void {
  // ERROR
  $x = (<<__OwnedMutable>> A $c) ==> {};
}

interface A {}

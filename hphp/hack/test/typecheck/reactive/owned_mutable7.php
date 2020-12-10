<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

function f(): void {
  // ERROR
  $z = function(<<__OwnedMutable>> A $c): void {
  };
}

interface A {}

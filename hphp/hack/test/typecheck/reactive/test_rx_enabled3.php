<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

<<__Rx>>
function f()[rx]: void {
  nonrx(); // should error
}

<<__Rx>>
function rx()[rx]: void {
}

function nonrx(): void {
}

<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

<<__Pure>>
function pure(): void {}

<<__Rx>>
function rx(): void {
  pure();
}

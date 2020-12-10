<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

<<__Rx>>
function rx(): void {}

<<__Pure>>
function pure(): void {
  rx();
}

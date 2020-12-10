<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

<<__RxShallow>>
function f(): void {
  // should be error
  echo 1;
}

<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

<<__RxShallow>>
function f()[rx_shallow]: void {
  // should be error
  echo 1;
}

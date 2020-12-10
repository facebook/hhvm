<?hh //strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

function nonrx(): int {
  return 1;
}

<<__RxLocal>>
function rxlocal(): int {
  return 1;
}

<<__RxShallow>>
function rxshallow(): int {
  // not OK - non-reactive calls are prohibited
  return nonrx();
}

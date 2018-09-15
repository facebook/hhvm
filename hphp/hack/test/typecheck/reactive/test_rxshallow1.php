<?hh //strict

<<__Rx>>
function rx(): int {
  return 1;
}

<<__RxLocal>>
function rxlocal(): int {
  return 1;
}

<<__RxShallow>>
function rxshallow(): int {
  // OK - reactive, reactive local and shallow are permitted
  return rx() + rxlocal() + rxshallow();
}

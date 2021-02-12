<?hh //strict

function rx(): int {
  return 1;
}


function rxlocal(): int {
  return 1;
}


function rxshallow(): int {
  // OK - reactive, reactive local and shallow are permitted
  return rx() + rxlocal() + rxshallow();
}

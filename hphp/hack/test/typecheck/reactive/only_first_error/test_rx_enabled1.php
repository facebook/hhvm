<?hh // strict

function f(): int {
  if (HH\Rx\IS_ENABLED) {
    return rx();
  } else {
    return nonrx();
  }
}

<<__Rx>>
function rx() {
  return 1;
}

function nonrx() {
  return 1;
}

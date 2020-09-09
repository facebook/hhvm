<?hh // strict

<<__Pure>>
function f(): int {
  if (HH\Rx\IS_ENABLED) {
    return rx();
  } else {
    return nonrx();
  }
}

<<__Pure>>
function rx() {
  return 1;
}

function nonrx() {
  return 1;
}

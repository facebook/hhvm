<?hh // strict

function f(): int {
  if (HH\Rx\IS_ENABLED) {
    return rx();
  } else {
    return nonrx();
  }
}

<<__Rx>>
function rx(): int {
  return 1;
}

function nonrx(): int {
  return 1;
}

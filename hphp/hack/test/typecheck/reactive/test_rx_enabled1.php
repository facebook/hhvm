<?hh // strict
function f()[write_props]: int {
  if (Rx\IS_ENABLED) {
    return rx();
  } else {
    return nonrx();
  }
}

<<__Rx>>
function rx()[rx]: int {
  return 1;
}

function nonrx(): int {
  return 1;
}

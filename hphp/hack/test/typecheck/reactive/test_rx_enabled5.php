<?hh // strict

<<__Rx>>
function f1(): int {
  if (Rx\IS_ENABLED) {
    return rx();
  } else {
    return nonrx();
  }
}

<<__Rx>>
function f2(): int {
  return Rx\IS_ENABLED ? rx() : nonrx();
}


// ======== RxShallow ========
<<__RxShallow>>
function f3(): int {
  if (Rx\IS_ENABLED) {
    return rx() + rxshallow();
  } else {
    return nonrx();
  }
}

<<__RxShallow>>
function f4(): int {
  return Rx\IS_ENABLED ? rx() + rxshallow() : nonrx();
}

// ======== RxLocal ========
<<__RxLocal>>
function f5(): int {
  if (Rx\IS_ENABLED) {
    return rx();
  } else {
    return nonrx();
  }
}

<<__RxLocal>>
function f6(): int {
  return Rx\IS_ENABLED ? rx() : nonrx();
}

<<__Rx>>
function f7(): int {
  invariant(Rx\IS_ENABLED, "Host with Rx support expected.");
  return rx();
}

<<__Rx>>
function f8(): int {
  invariant(!Rx\IS_ENABLED, "Host with Rx support not expected.");
  return nonrx();
}

<<__Rx>>
function f9(): int {
  if (Rx\IS_ENABLED) {
    return 0;
  }
  return nonrx();
}

<<__Rx>>
function f10(): int {
  if (!Rx\IS_ENABLED) {
    return nonrx();
  }
  return rx();
}

<<__Rx>>
function rx(): int {
  return 1;
}

<<__RxShallow>>
function rxshallow(): int {
  return 1;
}

function nonrx(): int {
  return 1;
}

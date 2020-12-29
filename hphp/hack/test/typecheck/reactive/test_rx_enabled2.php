<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

<<__Rx>>
function f1()[rx]: int {
  if (Rx\IS_ENABLED) {
    return rx();
  } else {
    return nonrx();
  }
}

<<__Rx>>
function f2()[rx]: int {
  return Rx\IS_ENABLED ? rx() : nonrx();
}


// ======== RxShallow ========
<<__RxShallow>>
function f3()[rx_shallow]: int {
  if (Rx\IS_ENABLED) {
    return rx() + rxshallow();
  } else {
    return nonrx();
  }
}

<<__RxShallow>>
function f4()[rx_shallow]: int {
  return Rx\IS_ENABLED ? rx() + rxshallow() : nonrx();
}

// ======== RxLocal ========
<<__RxLocal>>
function f5()[rx_local]: int {
  if (Rx\IS_ENABLED) {
    return rx();
  } else {
    return nonrx();
  }
}

<<__RxLocal>>
function f6()[rx_local]: int {
  return Rx\IS_ENABLED ? rx() : nonrx();
}

<<__Rx>>
function f7()[rx]: int {
  invariant(Rx\IS_ENABLED, "Host with Rx support expected.");
  return rx();
}

<<__Rx>>
function f8()[rx]: int {
  invariant(!Rx\IS_ENABLED, "Host with Rx support not expected.");
  return nonrx();
}

<<__Rx>>
function f9()[rx]: int {
  if (Rx\IS_ENABLED) {
    return 0;
  }
  return nonrx();
}

<<__Rx>>
function f10()[rx]: int {
  if (!Rx\IS_ENABLED) {
    return nonrx();
  }
  return rx();
}

<<__Rx>>
function rx()[rx]: int {
  return 1;
}

<<__RxShallow>>
function rxshallow()[rx_shallow]: int {
  return 1;
}

function nonrx(): int {
  return 1;
}

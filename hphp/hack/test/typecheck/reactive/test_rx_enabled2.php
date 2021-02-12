<?hh // strict

function f1()[rx]: int {
  if (Rx\IS_ENABLED) {
    return rx();
  } else {
    return nonrx();
  }
}


function f2()[rx]: int {
  return Rx\IS_ENABLED ? rx() : nonrx();
}


// ======== RxShallow ========

function f3()[rx_shallow]: int {
  if (Rx\IS_ENABLED) {
    return rx() + rxshallow();
  } else {
    return nonrx();
  }
}


function f4()[rx_shallow]: int {
  return Rx\IS_ENABLED ? rx() + rxshallow() : nonrx();
}

// ======== RxLocal ========

function f5()[rx_local]: int {
  if (Rx\IS_ENABLED) {
    return rx();
  } else {
    return nonrx();
  }
}


function f6()[rx_local]: int {
  return Rx\IS_ENABLED ? rx() : nonrx();
}


function f7()[rx]: int {
  invariant(Rx\IS_ENABLED, "Host with Rx support expected.");
  return rx();
}


function f8()[rx]: int {
  invariant(!Rx\IS_ENABLED, "Host with Rx support not expected.");
  return nonrx();
}


function f9()[rx]: int {
  if (Rx\IS_ENABLED) {
    return 0;
  }
  return nonrx();
}


function f10()[rx]: int {
  if (!Rx\IS_ENABLED) {
    return nonrx();
  }
  return rx();
}


function rx()[rx]: int {
  return 1;
}


function rxshallow()[rx_shallow]: int {
  return 1;
}

function nonrx(): int {
  return 1;
}

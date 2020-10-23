<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

// see hhi/coeffect/contexts.hhi for mapping to capabilities

function non_rx_fn()[defaults]: void {
  rx_fn();
}

function rx_fn()[rx]: void {
  pure_fn();
  non_rx_fn(); // error (non_rx context requires different capabilities)
}

function pure_fn()[pure]: void {
  rx_fn(); // error (pure context lacks the Rx capability)
  purest_fn();
}

function purest_fn()[]: void {
  pure_fn(); // error, purest has capabilities {} but pure is {Server}
  purest_fn(); // ok
}

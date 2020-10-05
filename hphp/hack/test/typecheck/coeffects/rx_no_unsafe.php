<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

// see hhi/coeffect/contexts.hhi for mapping to capabilities

function non_rx_fn()[non_rx]: void {
  rx_fn();
}

function rx_fn()[rx]: void {
  pure_fn();
  non_rx_fn(); // error (non_rx context requires additional capabilities)
}

function pure_fn()[pure]: void {
  rx_fn(); // error (pure context lacks the Rx capability)
  purest_fn();
}

function purest_fn()[]: void {
  // currently, `pure` has no additional capabilities, so it's equivalent to []
  pure_fn(); // ok
}

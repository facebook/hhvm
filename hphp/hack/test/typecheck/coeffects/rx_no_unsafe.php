<?hh
// see hhi/coeffect/contexts.hhi for mapping to capabilities

function non_rx_fn()[defaults]: void {
  rx_fn();
}

function rx_fn()[oldrx]: void {
  pure_fn();
  non_rx_fn(); // error (non_rx context requires different capabilities)
}

function pure_fn()[]: void {
  rx_fn(); // error (pure context lacks the Rx capability)
  pure_fn();
}

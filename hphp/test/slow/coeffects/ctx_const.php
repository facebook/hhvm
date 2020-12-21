<?hh

interface BaseIntIterator {
  abstract const ctx C super [rx, output];
  function next()[this::C]: int;

  const ctx Clazy = [];
  function has_next()[this::Clazy]: bool;

  abstract const ctx Cdebug as [output];
  function debug_current()[this::Cdebug]: void;
}

interface RxIntIterator extends BaseIntIterator {
  abstract const ctx C super [rx] = [rx]; // tighten bound & provide default
  <<__Override>> abstract const ctx Cdebug as []; // tighten bound
}

function next_no_output_mono(RxIntIterator $rx_iter)[rx]: int {
  return $rx_iter->next();
}

function debug_curr_and_next_poly(RxIntIterator $it)[
  $it::C, $it::Cdebug // path-dependent contexts
]: int {
  $it->debug_current();
  return $it->next();
}

class PureIntIterator implements RxIntIterator {
  // TODO(coeffects): https://fburl.com/diffusion/xwrk6tns
  // <<__Override>> const ctx C = [];
  function next()[]: int { return 0; }

  function has_next()[]: bool { return true; }

  const ctx Cdebug = [];
  function debug_current()[this::Cdebug]: void {}
}

function call_has_next<T as BaseIntIterator>(T $it)[$it::C]: bool {
  return $it->has_next();
}

<<__EntryPoint>>
function prints_0_1_0()[rx, output]: void {
  $it = new PureIntIterator();
  echo next_no_output_mono($it);
  echo call_has_next($it);
  echo debug_curr_and_next_poly($it);
}

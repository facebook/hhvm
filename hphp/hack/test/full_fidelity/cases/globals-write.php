<?hh // partial

function g() {
  $GLOBALS['derp'] = null; // ok
  $GLOBALS = null; // not ok
}

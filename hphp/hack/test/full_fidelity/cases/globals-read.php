<?hh // partial


function g() {
  $y = $GLOBALS['foo']; // ok
  $x = $GLOBALS; // not ok
}

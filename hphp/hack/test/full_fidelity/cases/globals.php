<?hh // partial


function f($_) {}
function g() {
  f($GLOBALS['derp']); // ok
  f($GLOBALS); // not ok
}

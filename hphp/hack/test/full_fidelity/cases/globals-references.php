<?hh // partial


function f($_) {}
function g() {
  f(&$GLOBALS);
}

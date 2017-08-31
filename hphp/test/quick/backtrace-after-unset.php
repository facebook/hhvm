<?hh
function f($x) {
  include __FILE__;
}
if (isset($x)) {
  unset($x);
  $x = debug_backtrace();
} else {
  f(10);
  print("Did not crash\n");
}

<?php
function f() {
  echo "f() was called\n";
}
function g() {
  echo "g() was called\n";
}
function main() {
  $obj = null;
  $x = $obj?->foo(f())?->bar(g());
  var_dump($x);
  $obj = new stdClass();
  $x = $obj?->foo(f())?->bar(g());
  var_dump($x);
}
main();

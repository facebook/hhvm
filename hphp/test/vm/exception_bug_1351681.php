<?php
function doThrow() { throw new Exception("blah!"); }
function foo() {
  foreach (array(1, 2, 3) as $_) {
    doThrow();
  }
  try { echo "Hi\n"; } catch (Exception $ex) { echo "We should not reach here\n"; }
}

try {
  foo();
} catch (Exception $x) {
  echo "it's ok\n";
}

<?php
function foo() {
  global $foo;
  return $foo;
}
foo();
foreach ($GLOBALS as $k => $v) {
  echo "$k=>$v\n";
}

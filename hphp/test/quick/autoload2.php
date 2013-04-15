<?php
function __autoload($cls) {
  echo "__autoload $cls\n";
  if ($cls === 'C') {
    class C { public function foo() { } }
  }
}
$arr = array("C");
$obj = new $arr[0];


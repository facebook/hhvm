<?php


<<__EntryPoint>>
function main_1717() {
$vals = array(null, 0, false, array(), 'test');
foreach ($vals as $val) {
  try {
    $val->foo();
  }
 catch (BadMethodCallException $e) {
    echo "BadMethodCallException thrown\n";
  }
}
}

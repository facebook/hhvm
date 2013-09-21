<?php
$a = array("a","b","c");
foreach ($a as &$k=>$v) {
  var_dump($v);
}
?>
<?php
echo "\nReference to constant array\n";
foreach (array(1,2) as &$v) {
  var_dump($v);
}
?>
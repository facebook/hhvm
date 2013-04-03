<?php
foreach (array(1,2) as $k=>&$v) {
  var_dump($v);
}
?>
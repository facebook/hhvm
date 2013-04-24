<?php

$array = array(0 => array('world'));

$it = new RecursiveIteratorIterator(new RecursiveArrayIterator($array));
foreach($it as $key => $val) {
   var_dump($key, $val);
}

?>
<?php

$fp = fopen(__FILE__, 'r');

$array = array(1,2,3,4,5,6,7);

var_dump($array[$fp]);

echo "Done\n";
?>
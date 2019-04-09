<?php


<<__EntryPoint>>
function main_1524() {
$a = array(1,2,3);
 $b = array(4,5,6);
 $i = 1;
 $a[$i++] = $b[$i++];
 var_dump($a);
}

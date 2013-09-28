<?php

$a = array(10);
 function test() {
 global $a;
 unset($a[0]);
 var_dump($a);
}
var_dump($a);
 test();
 var_dump($a);

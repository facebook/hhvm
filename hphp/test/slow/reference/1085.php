<?php

$a = array();
 function &f() {
 global $a;
 return $a['b'];
}
 $b = &f();
 $b = 20;
 var_dump($a);

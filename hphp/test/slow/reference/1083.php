<?php

$a = 10;
 function &f() {
 global $a;
 return $a;
}
 $b = &f();
 $b = 20;
 var_dump($a);

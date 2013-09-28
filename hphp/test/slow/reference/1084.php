<?php

function &f() {
 $a = 10;
 return $a;
}
 $b = &f();
 $b = 20;
 var_dump($b);

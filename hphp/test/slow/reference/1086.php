<?php

function &f() {
 $a = array();
 return $a['b'];
}
 $b = &f();
 $b = 20;
 var_dump($b);

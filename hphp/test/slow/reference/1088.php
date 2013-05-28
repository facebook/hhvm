<?php

function test($a) {
 $a[1] = 10;
 $a['r'] = 20;
}
 $b = 5;
 $a = array('r' => &$b);
 $a['r'] = 6;
 test($a);
 var_dump($a);

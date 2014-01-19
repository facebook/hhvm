<?php

$f = function($a) {
 return $a;
 }
;
 var_dump($f('x'));
 apc_store('key', $f);
 $g = apc_fetch('key');
 var_dump($g);


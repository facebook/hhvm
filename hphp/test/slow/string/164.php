<?php

$a = array('x'=>'foo');
$b = 'qqq';
class c {}
$c = new c;
$c->p = 'zzz';
var_dump("AAA ${a['x']} $a[x] $b $c->p");

<?php
$o = new StdClass();
$a = new StdClass();

$o->a = $a;

$so = new SplObjectStorage();

$so[$o] = 1;
$so[$a] = 2;

$s = serialize($so);
echo $s."\n";

$so1 = unserialize($s);
var_dump($so1);

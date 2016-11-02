<?php
// Source php weakref extension
$r = new StdClass;
$wr1 = new WeakRef($r);
var_dump($wr1->valid());
unset($wr1);
$wr2 = new WeakRef($r);
var_dump($wr2->valid());
unset($wr2);

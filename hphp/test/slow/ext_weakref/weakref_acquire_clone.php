<?php
// Source php weakref extension
$r = new StdClass;
$wr1 = new WeakRef($r);
var_dump($wr1->acquire());
unset($r);
$wr2 = clone $wr1;
var_dump($wr1->release());
var_dump($wr2->valid());

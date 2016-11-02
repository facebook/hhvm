<?php
// Source php weakref extension
$r = new StdClass;
$wr1 = new WeakRef($r);
var_dump($wr1->acquire());
unset($r);

<?php

// Source php weakref extension
<<__EntryPoint>>
function main_weakref_acquire() {
$r = new StdClass;
$wr1 = new WeakRef($r);
var_dump($wr1->acquire());
unset($r);
}

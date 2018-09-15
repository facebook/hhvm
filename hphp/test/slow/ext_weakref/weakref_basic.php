<?php

// Source php weakref extension
<<__EntryPoint>>
function main_weakref_basic() {
$o = new StdClass;
$wr = new WeakRef($o);
var_dump($wr->valid(), $wr->get());
unset($o);
var_dump($wr->valid(), $wr->get());
}

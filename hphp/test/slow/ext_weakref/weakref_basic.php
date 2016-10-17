<?php
// Source php weakref extension
$o = new StdClass;
$wr = new WeakRef($o);
var_dump($wr->valid(), $wr->get());
unset($o);
var_dump($wr->valid(), $wr->get());

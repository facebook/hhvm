<?php
// Source php weakref extension
$o = new StdClass;
$wr = new WeakRef($o);
$wr->acquire();
$wr->acquire();
var_dump($wr->valid(), $wr->get());
unset($o);
$wr->release();
var_dump($wr->valid(), $wr->get());
$wr->release();
var_dump($wr->valid(), $wr->get());

<?php

$a = array(1, 2, 3);
$o = new ArrayIterator($a);
var_dump($o->next());
var_dump($o->rewind());
var_dump($o->seek());
var_dump($o->asort());
var_dump($o->ksort());
var_dump($o->natsort());
var_dump($o->natcasesort());

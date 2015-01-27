<?php

$incomplete = new __php_incomplete_class();

$o = new SplObjectStorage();
$o->attach($incomplete);
var_dump($o->count());


$arr = new ArrayObject($incomplete); // No Exception

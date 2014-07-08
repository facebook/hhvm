<?php

$I = new IteratorIterator(new ArrayIterator(array(1,2,3)));
$I->rewind();
var_dump($I->current());
$I->next();
var_dump($I->current());
$I->next();
var_dump($I->current());
$I->next();
var_dump($I->current());

<?php


$v = new Vector;
$v[] = 123;
var_dump($v->toArray());
var_dump($v->pop());
var_dump($v->toArray());

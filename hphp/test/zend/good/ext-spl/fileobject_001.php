<?php

$o = new SplFileObject(dirname(__FILE__) . '/fileobject_001a.txt');

var_dump($o->key());
var_dump($o->current());
$o->setFlags(SplFileObject::DROP_NEW_LINE);
var_dump($o->key());
var_dump($o->current());
var_dump($o->key());
$o->next();
var_dump($o->key());
var_dump($o->current());
var_dump($o->key());
$o->rewind();
var_dump($o->key());
var_dump($o->current());
var_dump($o->key());
$o->seek(4);
var_dump($o->key());
var_dump($o->current());
var_dump($o->key());

echo "===A===\n";
foreach($o as $n => $l)
{
	var_dump($n, $l);
}

echo "===B===\n";
$o = new SplFileObject(dirname(__FILE__) . '/fileobject_001b.txt');
$o->setFlags(SplFileObject::DROP_NEW_LINE);
foreach($o as $n => $l)
{
	var_dump($n, $l);
}

?>
===DONE===
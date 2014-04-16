<?php

$m = new Memcached();
$m->addServer('localhost', 11211);

$obj = new stdclass;
$obj->int = 99;
$obj->string = 'a simple string';
$obj->array = array(11, 22);

$m->set('int', 99);
$m->set('string', 'a simple string');
$m->set('array', array(11, 12));
$m->set('object', $obj);

var_dump($m->getAllKeys());

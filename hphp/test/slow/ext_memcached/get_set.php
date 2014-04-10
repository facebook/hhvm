<?php

$memc = new Memcached();
$memc->addServer('localhost', '11211');

$key = 'foo';
$value = array('foo' => 'bar');

$memc->set($key, $value, 60);
var_dump($memc->get($key));

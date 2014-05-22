<?php

$memc = new Memcached();
$memc->addServer('localhost', '11211');

$key = 'delete_test';

var_dump($memc->set($key, 'foo', 60));
var_dump($memc->delete($key));
var_dump($memc->get($key));

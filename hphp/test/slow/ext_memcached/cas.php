<?php

$memc = new Memcached();
$memc->addServer('localhost', '11211');

$key = 'cas_test';
var_dump($memc->set($key, 10, 60));

var_dump($memc->get($key, null, $cas));

var_dump(is_double($cas));
var_dump($memc->cas($cas, $key, 11, 60));

var_dump($memc->get($key));

var_dump($memc->cas($cas, $key, 12, 60));

var_dump($memc->get($key));

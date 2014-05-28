<?php
$memcache = new Memcache();
$memcache->addServer('127.0.0.1', 11211, true);

$key = "key";
$value = "value";
var_dump($memcache->set($key, $value));
var_dump($memcache->get($key));

$key= "key with space";
$value = "value";
var_dump($memcache->set($key, $value));
var_dump($memcache->get($key));

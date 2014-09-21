<?php

$key = 'TEST_KEY_RETURN_TYPES';

$memcache = new Memcache;
$memcache->addServer('127.0.0.1', 11211);

$memcache->delete($key);

$string = "Some string";
$memcache->set($key, $string);
var_dump($memcache->get($key));

$integer = 1234567;
$memcache->set($key, $integer);
var_dump($memcache->get($key));

$double = 120.88;
$memcache->set($key, $double);
var_dump($memcache->get($key));

$boolean = true;
$memcache->set($key, $boolean);
var_dump($memcache->get($key));

$memcache->delete($key);

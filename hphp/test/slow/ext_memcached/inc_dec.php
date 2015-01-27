<?php
const key = 'incr_decr_test';
$mc = new Memcached;
$mc->addServer('127.0.0.1', 11211);
$mc->set(key, 0);
var_dump($mc->get(key));
$mc->increment(key, 3);
var_dump($mc->get( key));
$mc->decrement(key, 1);
var_dump($mc->get(key));

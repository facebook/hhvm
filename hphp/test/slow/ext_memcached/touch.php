<?php

$mc = new Memcached();
$mc->addServer('localhost', '11211');

$key = uniqid("this_does_not_exist_");

$mc->touch($key, 5);
var_dump($mc->getResultCode() == Memcached::RES_NOTFOUND);
$mc->set($key, 1, 5);

$mc->set($key, 1, 5);
var_dump($mc->getResultCode() == Memcached::RES_SUCCESS);

echo "OK\n";

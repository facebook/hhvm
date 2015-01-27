<?php
$mc = new Memcached;
$mc->addServer('127.0.0.1', 11211);
$mc->addServer('127.0.0.1', 11212);
for ($i = 1; $i <= 10; $i++) {
    $key = "dec_test_{$i}";
    $mc->set($key, 10);
    var_dump($mc->get($key) === 10);
    $mc->decrement($key, 1);
    var_dump($mc->get($key) === 9);
}

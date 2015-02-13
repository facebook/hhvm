<?php
require __DIR__ . '/redis.inc';

$key1 = 'testkey1';
$value = '6379';

// setup test
$r1 = NewRedisTestInstance();
$r1->setOption(Redis::OPT_PREFIX, GetTestKeyName(__FILE__) . ':');
$r2 = NewRedisTestInstance();
$r2->setOption(Redis::OPT_PREFIX, GetTestKeyName(__FILE__) . ':');

var_dump($r1->set($key1, $value));
var_dump($r1->watch($key1));
$r1->multi();
$r1->del($key1);
$r2->set($key1, 'different value');
var_dump($r1->exec());

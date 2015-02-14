<?php

include (__DIR__ . '/redis.inc');

$r = NewRedisTestInstance();
$r->setOption(Redis::OPT_PREFIX, GetTestKeyName(__FILE__) . ':');
$r->delete('A');

$r->zAdd('A', 1.2, 'one');
$r->zAdd('A', 1.4, 'two');
$r->zAdd('A', 1.1, 'three');
var_dump($r->zRangeByScore('A', '-inf', '+inf'));
var_dump($r->zRevRangeByScore('A', '+inf', '-inf'));
var_dump($r->zRangeByScore('A', 1.2, 1.3));
var_dump($r->zRevRangeByScore('A', 1.3, 1.2));

$r->delete('A');

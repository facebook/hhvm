<?php

include (__DIR__ . '/redis.inc');

$r = NewRedisTestInstance();
$r->setOption(Redis::OPT_PREFIX, GetTestKeyName(__FILE__) . ':');
$r->delete('A');

$r->zAdd('A', 1, 'one');
$r->zAdd('A', 2, 'two');
$r->zAdd('A', 3, 'three');
var_dump($r->zRange('A', 0, -1));
var_dump($r->zRevRange('A', 0, -1));

$r->delete('A');

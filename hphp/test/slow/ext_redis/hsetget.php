<?php

include (__DIR__ . '/redis.inc');

$r = NewRedisTestInstance();
$r->setOption(Redis::OPT_PREFIX, GetTestKeyName(__FILE__) . ':');
$r->delete('A');

$r->hSet('A', 'foo', 'bar');
var_dump($r->hLen('A'));
$r->hSet('A', 'baz', 'boom');
var_dump($r->hLen('A'));
var_dump($r->hKeys('A'));
var_dump($r->hVals('A'));
var_dump($r->hGetAll('A'));
var_dump($r->hGet('A', 'foo'));
var_dump($r->hGet('A', 'baz'));

$r->delete('A');

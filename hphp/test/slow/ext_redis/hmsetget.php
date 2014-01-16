<?php

include (__DIR__ . '/redis.inc');

$r = NewRedisTestInstance();
$r->setOption(Redis::OPT_PREFIX, GetTestKeyName(__FILE__) . ':');
$r->delete('A');

$r->hMSet('A', ['foo' => 'bar']);
var_dump($r->hLen('A'));
$r->hMSet('A', ['baz1' => 'boom1', 'baz2' => 'boom2']);
var_dump($r->hLen('A'));
var_dump($r->hKeys('A'));
var_dump($r->hVals('A'));
var_dump($r->hGetAll('A'));
var_dump($r->hGet('A', 'foo'));
var_dump($r->hGet('A', 'baz1'));
var_dump($r->hGet('A', 'baz2'));
var_dump($r->hGet('A', 'nofield'));

var_dump($r->hMGet('A', ['foo', 'baz1', 'baz2', 'nofield']));


$r->delete('A');

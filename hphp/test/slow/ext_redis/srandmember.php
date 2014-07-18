<?php

include (__DIR__ . '/redis.inc');

$r = NewRedisTestInstance();
$r->setOption(Redis::OPT_PREFIX, GetTestKeyName(__FILE__) . ':');
$r->delete('foo');
$r->sAdd('foo', 'bar');

var_dump($r->srandmember('foo'));
var_dump($r->srandmember('foo', 0));
var_dump($r->srandmember('foo', 1));

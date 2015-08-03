<?php

include (__DIR__ . '/redis.inc');

$r = NewRedisTestInstance();
$r->setOption(Redis::OPT_PREFIX, GetTestKeyName(__FILE__) . ':');
$r->delete('sscan');

$r->sadd('sscan', 'member:one', 'member:two', 'member:three', 'member:four');

var_dump($r->sscan('sscan', 0));
var_dump($r->sscan('sscan', 0, 'member:t*'));
var_dump($r->sscan('sscan', 0, 'nomember:*'));

$r->delete('sscan');

<?php

include (__DIR__ . '/redis.inc');

$r = NewRedisTestInstance();
$r->setOption(Redis::OPT_PREFIX, GetTestKeyName(__FILE__) . ':');
$r->delete('A');

$r->lpush('A', "abc", "easy as 123", "apple", "pear");
var_dump($r->lSize('A'));
var_dump($r->lRange('A', 1, 3));
var_dump($r->lpop('A'));
var_dump($r->rpop('A'));
var_dump($r->lSize('A'));

var_dump($r->rpoplpush('A', 'B'));
var_dump($r->lget('A', 0));
var_dump($r->lget('B', 0));

$r->delete('A', 'B');

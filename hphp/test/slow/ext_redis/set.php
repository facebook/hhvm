<?php

include (__DIR__ . '/redis.inc');

$r = NewRedisTestInstance();
$r->setOption(Redis::OPT_PREFIX, GetTestKeyName(__FILE__) . ':');

$r->delete('colors','reds');
var_dump($r->sAdd('colors', 'red', 'orange', 'yellow',
                            'green', 'blue', 'violet'));
var_dump($r->sSize('colors'));
var_dump($r->sContains('colors', 'red'));
var_dump($r->sContains('colors', 'aqua'));

var_dump($r->sAdd('reds', 'red', 'pink', 'crimson'));
var_dump($r->sInterStore('red-colors', 'colors', 'reds'));
var_dump($r->sMembers('red-colors'));

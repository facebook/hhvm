<?php

include (__DIR__ . '/redis.inc');

$r = NewRedisTestInstance();
$r->setOption(Redis::OPT_PREFIX, GetTestKeyName(__FILE__) . ':');
$r->delete('scan');

$r->mset(array('key:one' => 'one', 'key:two' => 'two',
               'key:three' => 'three','key:four' => 'four'));

var_dump($r->scan(0));
var_dump($r->scan(0, 'key:t*'));
var_dump($r->scan(0, 'nokey:t*'));

$r->delete('key:one');
$r->delete('key:two');
$r->delete('key:three');
$r->delete('key:four');

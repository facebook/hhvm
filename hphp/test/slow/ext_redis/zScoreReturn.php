<?php

include (__DIR__ . '/redis.inc');

$redis = NewRedisTestInstance();
$connect = $redis->connect('127.0.0.1',6379);
var_dump($connect);
$redis->zAdd('myzset', 1, 'one');
$result = $redis->zScore('myzset',"one");
var_dump($result);
$result = $redis->zScore('myzset',"one3");
var_dump ($result);

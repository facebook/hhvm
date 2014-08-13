<?php

include (__DIR__ . '/redis.inc');

$r = NewRedisTestInstance();
$r->setOption(Redis::OPT_PREFIX, GetTestKeyName(__FILE__) . ':');

$key = "test_lrem";
$r->delete($key);
$r->rPush($key, "A");
$r->rPush($key, "B");
$r->rPush($key, "C");

var_dump($r->lGet($key, 0));   // A
var_dump($r->lGet($key, 1));   // B
var_dump($r->lGet($key, 2));   // C
var_dump($r->lGet($key, -1));  // C

var_dump($r->lIndex($key, 0));   // A
var_dump($r->lIndex($key, 1));   // B
var_dump($r->lIndex($key, 2));   // C

var_dump($r->lRemove($key, "B", 1));  // 1 item req, 1 rm'ed
var_dump($r->lRemove($key, "B", 1));  // 1 item req, 0 rm'ed (it's already gone)

var_dump($r->lIndex($key, 0));   // A
var_dump($r->lIndex($key, 1));   // C

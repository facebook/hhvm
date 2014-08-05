<?php

include (__DIR__ . '/redis.inc');

$r = NewRedisTestInstance();
$r->setOption(Redis::OPT_PREFIX, GetTestKeyName(__FILE__) . ':');

$key = "test_lrem";
$r->delete($key);
$r->rPush($key, "A");
$r->rPush($key, "B");
$r->rPush($key, "C");

var_Dump($r->lGet($key, 0));   // A
var_Dump($r->lGet($key, 1));   // B
var_Dump($r->lGet($key, 2));   // C
var_Dump($r->lGet($key, -1));  // C

var_Dump($r->lIndex($key, 0));   // A
var_Dump($r->lIndex($key, 1));   // B
var_Dump($r->lIndex($key, 2));   // C

var_Dump($r->lRemove($key, "B", 1));  // 1 item requested, 1 removed
var_Dump($r->lRemove($key, "B", 1));  // 1 item requested, 0 removed (it's already gone)

var_Dump($r->lIndex($key, 0));   // A
var_Dump($r->lIndex($key, 1));   // C


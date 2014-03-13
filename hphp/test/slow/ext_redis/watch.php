<?php
require __DIR__ . '/redis.inc';

$key1 = 'testkey1';
$value = '6379';

// setup test
$r1 = NewRedisTestInstance();
$r1->setOption(Redis::OPT_PREFIX, GetTestKeyName(__FILE__) . ':');
$r2 = NewRedisTestInstance();
$r2->setOption(Redis::OPT_PREFIX, GetTestKeyName(__FILE__) . ':');

$r1->delete($key1);

var_dump($r1->set($key1, $value)); //true
var_dump($r1->watch($key1)); //true
$checkValue = $r1->get($key1);
var_dump($checkValue); //6379
if ($checkValue !== null && $checkValue == $value) {
  var_dump($r1->multi()->del($key1)->exec()); //[1]
}

var_dump($r1->set($key1, $value));
var_dump($r1->watch($key1));
$checkValue = $r1->get($key1);
var_dump($r2->set($key1, 'different value'));
var_dump($checkValue);
if ($checkValue !== null && $checkValue == $value) {
  var_dump($r1->multi()->del($key1)->exec());
}

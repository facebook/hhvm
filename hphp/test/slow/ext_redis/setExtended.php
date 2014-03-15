<?php

require __DIR__ . '/redis.inc';

// setup test
$r = NewRedisTestInstance();
$r->setOption(Redis::OPT_PREFIX, GetTestKeyName(__FILE__) . ':');
$r->delete('test1', 'test2', 'test3', 'test4', 'test5', 'test6', 'test7');

// normal sets
var_dump($r->set('test1', '1')); // true
var_dump($r->set('test2', '2', 5)); // true

// true if not exist
var_dump($r->set('test3', '3', ['nx'])); // true
var_dump($r->set('test3', '4', ['nx'])); // false

// true if exist
var_dump($r->set('test3', '5', ['xx'])); // true
var_dump($r->set('test4', '6', ['xx'])); // false

// true if not exist with expiration
var_dump($r->set('test4', '7', ['nx', 'ex' => 5])); // true
sleep(6);
var_dump($r->get('test4')); // null

// true if not exist with expiration
var_dump($r->set('test5', '8')); // true
var_dump($r->set('test5', '9', ['xx', 'ex' => 5])); // true
sleep(6);
var_dump($r->get('test5')); // null

// test px (millisecond expiration time)
var_dump($r->set('test6', '10', ['px' => 500])); //  true
var_dump($r->get('test6')); // "10"
sleep(1);
var_dump($r->get('test6')); // null

// test passing not exist and exist at the same time
try {
  $r->set('test7', '11', ['nx', 'xx']);
} catch (RedisException $e) {
  echo 'Redis Exception: ' . $e->getMessage() . PHP_EOL;
}

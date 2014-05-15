<?php

include(__DIR__ . '/redis.inc');

$r = NewRedisTestInstance();
$key = GetTestKeyName(__FILE__);
$r->delete($key);
echo "incr/decr Atomic\n";
var_dump($r->incr($key));
var_dump($r->incr($key, 2));
var_dump($r->incrBy($key, 3));
var_dump($r->decr($key));
var_dump($r->decr($key, 2));
var_dump($r->decrBy($key, 3));
echo "incr/decr Multi\n";
var_dump(
  $r->multi()
    ->incr($key)
    ->incr($key, 2)
    ->incrBy($key, 3)
    ->decr($key)
    ->decr($key, 2)
    ->decrBy($key, 3)
    ->exec()
);
echo "incr/decr Pipeline\n";
var_dump(
  $r->pipeline()
    ->incr($key)
    ->incr($key, 2)
    ->incrBy($key, 3)
    ->decr($key)
    ->decr($key, 2)
    ->decrBy($key, 3)
    ->exec()
);

$r->delete($key);
echo "incrByFloat Atomic\n";
var_dump($r->incrByFloat($key, 1.5));
var_dump($r->incrByFloat($key, -1.5));
echo "incrByFloat Multi\n";
var_dump($r->multi()
  ->incrByFloat($key, 1.5)
  ->incrByFloat($key, -1.5)
  ->exec());
echo "incrByFloat Pipeline\n";
var_dump($r->pipeline()
  ->incrByFloat($key, 1.5)
  ->incrByFloat($key, -1.5)
  ->exec());

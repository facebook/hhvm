<?php

include(__DIR__ . '/redis.inc');

$r = NewRedisTestInstance();
$key = GetTestKeyName(__FILE__);
$r->delete($key);

// eval
echo "eval\n";
var_dump($r->eval("return redis.call('set',KEYS[1],ARGV[1])", [$key,'bar'], 1)); // With parameters -> OK
var_dump($r->eval("return redis.call('get','{$key}')")); // Without parameters -> bar

// evalSha
echo "evalSha\n";
var_dump($r->evalSha("c686f316aaf1eb01d5a4de1b0b63cd233010e63d",[$key,'bar'], 1)); // SHA1SUM of with parameters -> OK
var_dump($r->evalSha("ffffffffffffffffffffffffffffffffffffffff")); // No SHA1SUM -> NOSCRIPT error

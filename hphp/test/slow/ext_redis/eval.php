<?php

include (__DIR__ . '/redis.inc');

$r = NewRedisTestInstance();
$prefix = GetTestKeyName(__FILE__).':';
$r->setOption(Redis::OPT_PREFIX, $prefix);

foreach (['eval', 'evaluate'] as $method) {
    var_dump($r->$method('return 42'));
    var_dump($r->$method('return {1,2,{3,4,{"a","b"}}}'));
    var_dump($r->$method('return redis.call("set", KEYS[1], "bar")', ['foo'], 1));

    $r->setOption(Redis::OPT_PREFIX, 'ext_redis_eval_test:');
    var_dump($r->$method('return {KEYS[1],KEYS[2],ARGV[1],ARGV[2]}', ['key1', 'key2', 'first', 'second'], 2));
    $r->setOption(Redis::OPT_PREFIX, $prefix);
}

$sha = $r->script('load', 'return 42');
var_dump($r->evalSha($sha));
var_dump($r->evaluateSha($sha));

<?php

include(__DIR__ . '/redis.inc');

$r = NewRedisTestInstance();
$key = GetTestKeyName(__FILE__);
$prefix = $key . ':';
$r->delete($key);
$r->setOption(Redis::OPT_PREFIX, $prefix);

foreach (['eval', 'evaluate'] as $method) {
    echo $method . "\n";
    var_dump($r->$method('return 42')); // Return integer -> 42
    // Return results as array()
    var_dump($r->$method('return {1,2,{3,4,{"a","b"}}}'));
    // Script with parameters -> OK
    var_dump($r->eval("return redis.call('set',KEYS[1],ARGV[1])",
                      [$key, 'bar'], 1));

    $r->setOption(Redis::OPT_PREFIX, 'ext_redis_eval_test:');
    var_dump($r->$method('return {KEYS[1],KEYS[2],ARGV[1],ARGV[2]}',
                         ['key1', 'key2', 'first', 'second'], 2));
    $r->setOption(Redis::OPT_PREFIX, $prefix);
}

$sha = $r->script('load', 'return 42');
foreach (['evalSha', 'evaluateSha'] as $method) {
    echo $method . "\n";
    var_dump($r->$method($sha)); // Return integer -> 42
    // SHA1SUM of with parameters -> OK
    var_dump($r->$method("c686f316aaf1eb01d5a4de1b0b63cd233010e63d",
                         [$key, 'bar'], 1));
    $r->clearLastError();
    // No SHA1SUM -> NOSCRIPT error
    var_dump($r->$method("ffffffffffffffffffffffffffffffffffffffff"));
    var_dump(gettype($r->getLastError()));
}

echo "script\n";
var_dump($r->script('load','return 42'));
var_dump($r->script('load','bad script'));
var_dump($r->script('exists','1fa00e76656cc152ad327c13fe365858fd7be306',
                    'ffffffffffffffffffffffffffffffffffffffff'));
var_dump($r->script('flush'));
var_dump($r->script('kill'));
var_dump(gettype($r->getLastError()));
$r->clearLastError();
var_dump(gettype($r->getLastError()));

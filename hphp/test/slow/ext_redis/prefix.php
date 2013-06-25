<?php

include (__DIR__ . '/redis.inc');

$r = NewRedisTestInstance();
$key = GetTestKeyName(__FILE__);

var_dump($r->setOption(Redis::OPT_PREFIX, "{$key}:"));
var_dump($r->set("foo", "bar{$key}baz"));
var_dump($r->setOption(Redis::OPT_PREFIX, ""));
var_dump($r->get("{$key}:foo") == "bar{$key}baz");
var_dump($r->delete("{$key}:foo"));

<?php

include (__DIR__ . '/redis.inc');

$r = NewRedisTestInstance();
$tm = $r->time();
var_dump(count($tm));
$tm = $tm[0] + ($tm[1] / 1000000);
$now = microtime(true);
$delta = abs($now - $tm);
// Server should be within five seconds of client
var_dump($delta < 5);

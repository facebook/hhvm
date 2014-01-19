<?php
require(__DIR__ . '/redis.inc');

$r = NewRedisTestInstance(true);
$info = $r->info();
var_dump(is_array($info));
var_dump(count($info) > 35);
var_dump($info['tcp_port'] == REDIS_PORT);
$r->client('setname', 'hhvm-redis-client');
var_dump($r->client('getname'));

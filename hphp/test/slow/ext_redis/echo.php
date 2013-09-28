<?php

include (__DIR__ . '/redis.inc');

$r = NewRedisTestInstance();
var_dump($r->echo("This is a test"));

$r->multi(Redis::MULTI);
var_dump($r->echo("This is a multi test") instanceof Redis);
var_dump($r->exec());

$r->multi(Redis::PIPELINE);
var_dump($r->echo("This is a pipeline test") instanceof Redis);
var_dump($r->exec());

<?php

include (__DIR__ . '/redis.inc');

$r = NewRedisTestInstance();
$r->setOption(Redis::OPT_PREFIX, GetTestKeyName(__FILE__) . ':');
$r->setOption(Redis::OPT_SERIALIZER, Redis::SERIALIZER_PHP);

foreach ([null, true, false, 123, 456.0,
          "A string of words", [1,2,3]] as $val) {
  $r->set('A', $val);
  var_dump($r->get('A'));
}

$r->delete('A');

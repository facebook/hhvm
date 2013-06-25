<?php

include (__DIR__ . '/redis.inc');

$r = NewRedisTestInstance();
$r->setOption(Redis::OPT_PREFIX, GetTestKeyName(__FILE__) . ':');

$r->set("A", "\x01\x02\x03\x04");
$r->set("B", "\x01\x01\x01\x01");

foreach (['AND', 'OR', 'XOR'] as $op) {
  $r->bitop($op, 'C', 'A', 'B');
  var_dump(bin2hex($r->get('C')));
}

$r->bitop('NOT', 'C', 'A');
var_Dump(bin2hex($r->get('C')));


$r->delete('A', 'B', 'C');

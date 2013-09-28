<?php

include (__DIR__ . '/redis.inc');

$r = NewRedisTestInstance();
$key = GetTestKeyName(__FILE__);
$r->delete($key);

echo "Atomic\n";
var_dump($r->exists($key));
var_dump($r->set($key, "The quick brown fox jumped over the lazy dog."));
var_dump($r->exists($key));
var_dump($r->get($key));
var_dump($r->strlen($key));
var_dump($r->delete($key));
var_dump($r->exists($key));

echo "Multi\n";
var_dump($r->multi()
           ->exists($key)
           ->set($key, "The quick brown fox jumped over the lazy dog.")
           ->exists($key)
           ->get($key)
           ->strlen($key)
           ->delete($key)
           ->exists($key)
           ->exec());

echo "Pipeline\n";
var_dump($r->pipeline()
           ->exists($key)
           ->set($key, "The quick brown fox jumped over the lazy dog.")
           ->exists($key)
           ->get($key)
           ->strlen($key)
           ->delete($key)
           ->exists($key)
           ->exec());


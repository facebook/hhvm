<?php

$m1 = new Memcached('id1');
$m1->setOption(Memcached::OPT_PREFIX_KEY, "foo_");
var_dump($m1->isPristine());
$m1 = new Memcached('id1');
var_dump($m1->isPristine());
$m2 = new Memcached('id1');
var_dump($m2->isPristine());
// this change affects $m1
$m2->setOption(Memcached::OPT_PREFIX_KEY, "bar_");
$m3 = new Memcached('id2');
var_dump($m3->isPristine());
$m3 = new Memcached();
var_dump($m3->isPristine());

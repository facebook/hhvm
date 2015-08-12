<?php

$m1 = new Memcached('id1');
$m1->setOption(Memcached::OPT_PREFIX_KEY, "foo_");
var_dump($m1->isPersistent());
$m1 = new Memcached('id1');
var_dump($m1->isPersistent());
$m2 = new Memcached('id1');
var_dump($m2->isPersistent());
// this change affects $m1
$m2->setOption(Memcached::OPT_PREFIX_KEY, "bar_");
$m3 = new Memcached('id2');
var_dump($m3->isPersistent());
$m3 = new Memcached();
var_dump($m3->isPersistent());
// objects have the same resource, but they are not the same object.
var_dump($m1 === $m2);
var_dump($m1->getOption(Memcached::OPT_PREFIX_KEY));

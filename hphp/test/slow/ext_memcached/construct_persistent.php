<?php

$memc1 = new Memcached("test");
$memc1->setOption(Memcached::OPT_PREFIX_KEY, 'php');
var_dump($memc1->getOption(Memcached::OPT_PREFIX_KEY));

$memc2 = new Memcached("test");
var_dump($memc2->getOption(Memcached::OPT_PREFIX_KEY));

$memc3 = new Memcached();
var_dump($memc3->getOption(Memcached::OPT_PREFIX_KEY));

<?php
$mc = new Memcached;
$mc->addServer('127.0.0.1', 11211);
$mc->setOption(Memcached::OPT_LIBKETAMA_COMPATIBLE, true);
var_dump($mc->getOption(Memcached::OPT_HASH) === Memcached::HASH_MD5);
var_dump($mc->getOption(Memcached::OPT_DISTRIBUTION) === Memcached::DISTRIBUTION_CONSISTENT_KETAMA);
$mc->setOption(Memcached::OPT_LIBKETAMA_COMPATIBLE, false);
var_dump($mc->getOption(Memcached::OPT_HASH) === Memcached::HASH_DEFAULT);
var_dump($mc->getOption(Memcached::OPT_DISTRIBUTION) === Memcached::DISTRIBUTION_MODULA);

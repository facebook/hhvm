<?php
$mc = new Memcached();
var_dump(Memcached::OPT_HASH_WITH_PREFIX_KEY);
var_dump($mc->getOption(Memcached::OPT_HASH_WITH_PREFIX_KEY));
var_dump($mc->setOption(Memcached::OPT_HASH_WITH_PREFIX_KEY, true));
var_dump($mc->getOption(Memcached::OPT_HASH_WITH_PREFIX_KEY));

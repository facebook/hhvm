<?php

function memcache_init_split_vars() {
  global $_SERVER;
  global $MEMCACHED_SPLIT_HASH;
  $MEMCACHED_SPLIT_HASH =    crc32(empty($_SERVER['SERVER_ADDR']) ? php_uname('n')                                         : $_SERVER['SERVER_ADDR']);
}

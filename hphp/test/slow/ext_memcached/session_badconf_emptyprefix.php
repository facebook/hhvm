<?php
// session_badconf_emptyprefix from pecl extension
ini_set('memcached.sess_prefix', 'test_memc.sess.key.');
ini_set('session.save_handler', 'memcached');
ini_set('session.save_path', 'localhost:11211');

error_reporting(0);
function handler($errno, $errstr) {
  echo "$errstr\n";
}

set_error_handler('handler', E_ALL);

// Empty prefix
ini_set('memcached.sess_prefix', " \n");
session_start();
session_write_close();

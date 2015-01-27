<?php
// session_basic.php from pecl extension tests
ini_set('memcached.sess_prefix', 'test_memc.sess.key.');
ini_set('session.save_handler', 'memcached');
ini_set('session.save_path', 'localhost:11211');

error_reporting(0);

session_start();
$_SESSION['foo'] = 1;
session_write_close();

$_SESSION = null;

var_dump($_SESSION);
session_start();
var_dump($_SESSION);
session_write_close();

session_start();
session_destroy();

session_start();
var_dump($_SESSION);
session_write_close();

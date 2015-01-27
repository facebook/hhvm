<?php
ini_set('memcached.sess_prefix', 'test_memc.sess.key.');
ini_set('session.save_handler', 'memcached');
ini_set('session.save_path', 'localhost:11211');

var_dump(session_start());
$id = session_id();
var_dump($id);

$key = ini_get('memcached.sess_prefix') . $id;

$_SESSION['_test_key'] = 'Test';

$memcache = new Memcached;
$memcache->addServer('localhost', '11211');

var_dump($memcache->get($key));
session_write_close();
var_dump($memcache->get($key));

var_dump(session_start());
session_write_close();

// Test destroy
var_dump(session_start());
var_dump(session_destroy());
var_dump($memcache->get($key));

// Custom prefix test
ini_set('memcached.sess_prefix', 'test_prefix');
var_dump(session_start());
$id = session_id();
var_dump($id);
$key = 'test_prefix' . $id;
$_SESSION['_test_key2'] = 'Test2';
var_dump($memcache->get($key));
session_write_close();
var_dump($memcache->get($key));

<?php
// This is test 36 from the pecl extension.  It tests session support.
$host = 'localhost';
$port = '11211';
$session_save_path
  = "tcp://$host:$port?persistent=1&udp_port=0&weight=2&timeout=2"
  . "&retry_interval=10,  ,tcp://$host:$port  ";

ini_set('session.save_handler', 'memcache');
ini_set('session.save_path', $session_save_path);

var_dump(session_start());
$id = session_id();
var_dump($id);

$_SESSION['_test_key'] = 'Test';

$memcache = new Memcache;
$memcache->addServer($host, $port);

var_dump($memcache->get($id));
session_write_close();
var_dump($memcache->get($id));

var_dump(session_start());
session_write_close();

// Test destroy
var_dump(session_start());
var_dump(session_destroy());
var_dump($memcache->get($id));

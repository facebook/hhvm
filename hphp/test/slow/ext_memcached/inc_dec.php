<?php
const key = 'incr_decr_test';
$mc = new Memcached;
$mc->addServer('127.0.0.1', 11211);
$mc->set(key, 0);
var_dump($mc->get(key));
$mc->increment(key, 3);
var_dump($mc->get( key));
$mc->decrement(key, 1);
var_dump($mc->get(key));

//increment with initial value only works with binary protocol
const non_existant_key = 'incr_decr_test_fail';

$mc2 = new Memcached;
$mc2->setOption(Memcached::OPT_BINARY_PROTOCOL, true);
$mc2->addServer('127.0.0.1', 11211);

var_dump($mc2->increment(non_existant_key, 3));
var_dump($mc2->get_multi(array(non_existant_key)));
var_dump($mc2->decrement(non_existant_key, 1));
var_dump($mc2->get_multi(array(non_existant_key)));

$mc2->increment(non_existant_key, 3, 1);
$result = $mc2->get_multi(array(non_existant_key));
var_dump($result[non_existant_key]);

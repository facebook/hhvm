<?php

require_once(dirname(__FILE__) . '/new_db.inc');

$func = 'strtoupper';
var_dump($db->createfunction($func, $func));
var_dump($db->querySingle('SELECT strtoupper("test")'));

$func2 = 'strtolower';
var_dump($db->createfunction($func2, $func2));
var_dump($db->querySingle('SELECT strtolower("TEST")'));

var_dump($db->createfunction($func, $func2));
var_dump($db->querySingle('SELECT strtoupper("tEst")'));


?>
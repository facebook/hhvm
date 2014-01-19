<?php
require_once __DIR__."/../utils/server.inc";
$m = mongo_standalone();
$c = $m->phpunit->col;
$c->drop();

// insert requires an array or object
$c->insert(); 
var_dump($c->findOne());
$c->drop();
$c->insert('hi'); 
var_dump($c->findOne());
$c->drop();
$c->insert(10); 
var_dump($c->findOne());
$c->drop();
$c->insert(false); 
var_dump($c->findOne());
$c->drop();

// will insert accept a numeric array?
$c->insert(array('foo','bar','baz'));
var_dump($c->findOne());
$c->drop();

// this should be fine
$obj = new StdClass();
$obj->hello = 'Hello, World!';
$c->insert($obj);
var_dump($c->findOne());
$c->drop();

// 2nd parameter should be array of options
$c->insert(array('yo'=>'ho'), true); 
var_dump($c->findOne());
$c->drop();

try
{
	$c->insert(array('yo2'=>'ho2'), array('invalid_option')); // I expect an exception here, but I don't get one?
	var_dump($c->findOne());
}
catch (Exception $e)
{
	echo $e->getMessage();
}
?>
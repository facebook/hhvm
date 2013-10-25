<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$db = $mongo->selectDB('test');

var_dump($db->getDBRef(array()));
var_dump($db->getDBRef(array('$ref' => 'dbref')));
var_dump($db->getDBRef(array('$id' => 123)));
?>
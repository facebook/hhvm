<?php
require_once __DIR__."/../utils/server.inc";
$m = mongo_standalone();
$db = $m->selectDB(dbname());
$db->dropCollection("addresses");
$c = $db->addresses;

$c->insert(array("stuff" => "bar", "zip-code" => 10010));
$c->insert(array("stuff" => "foo", "zip-code" => 10010));
$c->insert(array("stuff" => "bar", "zip-code" => 99701), array("safe" => true));

$retval = $c->distinct("zip-code");
var_dump($retval);

$retval = $c->distinct("zip-code", array("stuff" => "foo"));
var_dump($retval);

$retval = $c->distinct("zip-code", array("stuff" => "bar"));
var_dump($retval);

$c->insert(array("user" => array("points" => 25)));
$c->insert(array("user" => array("points" => 31)));
$c->insert(array("user" => array("points" => 25)), array("safe" => true));

$retval = $c->distinct("user.points");
var_dump($retval);
$retval = $c->distinct("user.nonexisting");
var_dump($retval);
?>
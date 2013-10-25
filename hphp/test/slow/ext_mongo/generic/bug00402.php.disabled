<?php
require_once __DIR__."/../utils/server.inc";

$m = mongo_standalone();
$c = $m->selectCollection('phpunit', 'col');
$c->insert(array('x' => 1), array('safe' => true));

$result = $c->validate();
var_dump(isset($result['warning']));
var_dump($result['warning']);

$result = $c->validate(true);
var_dump(isset($result['warning']));

$c->drop();
$res = $c->validate();
var_dump($res["ok"], $res["errmsg"]);
?>
<?php
require_once __DIR__."/../../utils/server.inc";

// The connect param will always be true in RS so we only test this standalone

$c = new Mongo("mongodb://$STANDALONE_HOSTNAME:$STANDALONE_PORT", array("connect" => false));
var_dump($c->connected);
$a = new Mongo("mongodb://$STANDALONE_HOSTNAME");
var_dump($a->connected);
$b = new Mongo("mongodb://$STANDALONE_HOSTNAME:$STANDALONE_PORT");
var_dump($b->connected);
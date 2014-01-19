<?php
require_once __DIR__."/../utils/server.inc";

$hostname = hostname();
$port     = standalone_port();

$a = new Mongo($hostname);
var_dump($a instanceof Mongo);
$b = new Mongo("$hostname:$port");
var_dump($b instanceof Mongo);
?>
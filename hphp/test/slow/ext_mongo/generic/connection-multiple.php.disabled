<?php
require_once __DIR__."/../utils/server.inc";

$hostname = hostname();
$port = standalone_port();
$ip = gethostbyname($hostname);

$a = new Mongo("$hostname,$ip");
var_dump($a->connected);
$b = new Mongo("$hostname:$port,$ip:$port");
var_dump($b->connected);
?>
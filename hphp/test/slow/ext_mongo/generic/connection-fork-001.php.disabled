<?php
require_once __DIR__."/../utils/server.inc";

$m = mongo_standalone();

$col = $m->selectDb("phpunit")->fork;

$col->drop();
$col->insert(array("parent" => time()), array("safe" => 1));

$pid = pcntl_fork();
if ($pid == 0) {
	$col->count();
	exit;
}

$n = 0;
while($n++ < 1000) {
	$col->insert(array("parent" => time()), array("safe" => 1));
}

echo $col->count(), "\n";
?>
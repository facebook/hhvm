<?php
require_once __DIR__."/../utils/server.inc";
//require_once dirname(__FILE__) . "/../debug.inc";

$m = mongo_standalone();

$col = $m->selectDb("phpunit")->fork;

$col->drop();
$col->insert(array("parent" => time()), array("safe" => 1));

$pid = pcntl_fork();
if ($pid == 0) {
	$n = 0;
	while($n++ < 1000) {
		$col->insert(array("parent" => time()), array("safe" => 1));
	}

	echo $col->count(), "\n";
} else {
	echo $col->count(), "\n";
}
?>
<?php
require_once __DIR__."/../utils/server.inc";

$host = MongoShellServer::getStandaloneInfo();
$mc = new MongoClient($host);
$db = $mc->test;

$collection = $db->part1->part2;
var_dump($collection->getName());

?>
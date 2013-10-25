<?php
require_once __DIR__."/../utils/server.inc";
$conn = new_mongo_standalone();
$db   = $conn->phpunit;

$grid = $db->getGridFS();

$grid->storeBytes('some thing', array('filename' => '1.txt'), array('safe' => true));

echo "No memory leak";
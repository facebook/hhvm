<?php
require_once __DIR__."/../utils/server.inc";
$dsn = MongoShellServer::getStandaloneInfo();
$conn = new MongoClient($dsn);
$db   = $conn->selectDb('phpunit');
$grid = $db->getGridFs('wrapper');

// delete any previous results
$grid->drop();

// dummy file
$bytes = "";
for ($i=0; $i < 200*1024; $i++) {
    $bytes .= sha1(rand(1, 1000000000));
}
$grid->storeBytes($bytes, array("filename" => "demo.txt"), array('safe' => true));

// fetch it
$file = $grid->findOne(array('filename' => 'demo.txt'));
$chunkSize = $file->file['chunkSize'];

// get file descriptor
$fp = $file->getResource();

$memory = memory_get_usage();

$tmp = "";
$i=0;
while (!feof($fp)) {
    $tmp .= ($t=fread($fp, rand(1024,8024)));
}
var_dump($bytes === $tmp);

// memory leak checks
var_dump((memory_get_usage() - $memory - strlen($tmp)) < $chunkSize * 1.5 );
fclose($fp);

var_dump((memory_get_usage() - $memory - strlen($tmp)) < $chunkSize *.5);
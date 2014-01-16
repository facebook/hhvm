<?php
require_once __DIR__."/../utils/server.inc";
$conn = mongo_standalone();
$db   = $conn->selectDb('phpunit');
$grid = $db->getGridFs('wrapper');

// delete any previous results
$grid->drop();

// dummy file
$bytes = "";
for ($i=0; $i < 200*1024; $i++) {
    $bytes .= sha1(rand(1, 1000000000));
}
$length = 200*1024 * 40;

$grid->storeBytes($bytes, array("filename" => "demo.txt"));

// fetch it
$file = $grid->findOne(array('filename' => 'demo.txt'));
$chunkSize = $file->file['chunkSize'];

// get file descriptor
$fp = $file->getResource();

// test sets
$tests = array(
	array(66061, 97588),
);

/* seek test */
$result = true;

$iter   = 5000;
for ($i=0; $i < $iter && $result; $i++) {
    $base   = rand(0, $chunkSize/2);
    $offset = rand(0, $chunkSize/2);

    fseek($fp, $base, SEEK_SET);
    $read     = fread($fp, 1024);
	$expected = substr($bytes, $base, 1024);
    if (strncmp($read, $expected, strlen($read))) {
        var_dump($base, $expected, $read);
        die("FAILED: SEEK_SET");
    }

    fseek($fp, $offset, SEEK_CUR);
	$read     = fread($fp, 1024);
	$expected = substr($bytes, $base + 1024 + $offset, 1024);
    if (strncmp($read, $expected, strlen($read))) {
        var_dump($base, $base + $offset, $expected, $read);
        die("FAILED: SEEK_CUR");
    }

    fseek($fp, -1*$base, SEEK_END);
	$read     = fread($fp, 1024);
	$expected = substr($bytes, $length - $base, 1024);
    if (strncmp($read, $expected, strlen($read))) {
        var_dump($length - $base, $expected, $read);
        die("FAILED: SEEK_END");
    }
}

var_dump($result && $i === $iter);
?>
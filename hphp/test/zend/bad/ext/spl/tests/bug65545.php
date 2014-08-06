<?php
$obj = new SplFileObject(__FILE__, 'r');
$data = $obj->fread(5);
var_dump($data);

$data = $obj->fread();
var_dump($data);

$data = $obj->fread(0);
var_dump($data);

// read more data than is available
$data = $obj->fread(filesize(__FILE__) + 32);
var_dump(strlen($data) === filesize(__FILE__) - 5);

?>

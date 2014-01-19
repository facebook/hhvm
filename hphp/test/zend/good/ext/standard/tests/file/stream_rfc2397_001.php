<?php
ini_set('allow_url_fopen', 1);


$data = 'data://,hello world';

var_dump(file_get_contents($data));

$file = fopen($data, 'r');
unset($data);

var_dump(stream_get_contents($file));

?>
===DONE===
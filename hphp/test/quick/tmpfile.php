<?php
$f = tmpfile();
if ($f === false)
    exit();

$path = stream_get_meta_data($f)['uri'];
var_dump($path);
var_dump(file_exists($path));

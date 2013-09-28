<?php
$fd = fopen('php://temp', 'r+');
$delimiter = 'MM';
$str = str_repeat('.', 8191) . $delimiter . "rest";
fwrite($fd, $str);
rewind($fd);
$line = stream_get_line($fd, 9000, $delimiter);
var_dump(strlen($line));
$line = stream_get_line($fd, 9000, $delimiter);
var_dump($line);
?>
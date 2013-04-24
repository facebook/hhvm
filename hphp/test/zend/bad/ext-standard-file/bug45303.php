<?php
$fd = fopen("php://stdout","a");
var_dump($fd);
var_dump(fseek($fd, 1024*1024, SEEK_SET));
?>
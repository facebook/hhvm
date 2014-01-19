<?php

$file = basename(__FILE__);

$fd = fopen($file, "r", true);
var_dump($fd);
fclose($fd);

?>
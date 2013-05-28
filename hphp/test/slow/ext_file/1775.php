<?php

error_reporting(0);
$fp = fopen('/tmp/lock.txt', 'w');
fclose($fp);
$fp = fopen('/tmp/lock.txt', 'r+');
var_dump(flock($fp, 0xf0));
fclose($fp);

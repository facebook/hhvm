<?php


<<__EntryPoint>>
function main_1775() {
error_reporting(0);
$tempfile = tempnam(sys_get_temp_dir(), 'lock');
$fp = fopen($tempfile, 'w');
fclose($fp);
$fp = fopen($tempfile, 'r+');
var_dump(flock($fp, 0xf0));
fclose($fp);
unlink($tempfile);
}

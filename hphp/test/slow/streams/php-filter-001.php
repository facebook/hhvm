<?php


<<__EntryPoint>>
function main_php_filter_001() {
$tmpfile = tempnam(sys_get_temp_dir(), basename(__FILE__));
$fp = fopen('php://filter/string.toupper/resource='.$tmpfile, 'w');
fwrite($fp, "Hello World\n");
fclose($fp);
readfile($tmpfile);
unlink($tmpfile);
}

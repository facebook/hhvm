<?php
ini_set('log_errors', On);

ini_set('date.timezone', ASIA/Chongqing);

$dir = dirname(__FILE__);
$log = $dir . "/tmp.err";
ini_set("error_log", $log);
echo $aa;
error_log("dummy");
readfile($log);
unlink($log);
?>
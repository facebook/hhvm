<?php
var_dump(pcntl_exec());
$file = tempnam(sys_get_temp_dir(),"php");
var_dump(pcntl_exec($file, array("foo","bar"), array("foo" => "bar")));
unlink($file);
?>
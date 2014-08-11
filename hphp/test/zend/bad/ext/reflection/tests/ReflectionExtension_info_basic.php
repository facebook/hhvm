<?php
$obj = new ReflectionExtension('reflection');
ob_start();
$testa = $obj->info();
$testb = ob_get_clean();
var_dump($testa);
var_dump(strlen($testb) > 24);
?>
==DONE==

<?php
$ext = new ReflectionExtension("standard");
$consts = $ext->getConstants();
var_dump($consts["CONNECTION_NORMAL"]);
?>

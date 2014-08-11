<?php
$ext = new ReflectionExtension("standard");
$funcs = $ext->getFunctions();
echo $funcs["sleep"]->getName();
?>

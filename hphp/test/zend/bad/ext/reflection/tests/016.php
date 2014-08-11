<?php
$ext = new ReflectionExtension("xml");
$deps = $ext->getDependencies();
var_dump($deps);
?>

<?php
$ext = new ReflectionExtension("reflection");
$classes = $ext->getClasses();
echo $classes["ReflectionException"]->getName();
?>

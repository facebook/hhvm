<?php

$rf = new ReflectionFunction('is_file');
var_dump($rf->isDisabled());
var_dump(method_exists('ReflectionFunction', 'isDisabled'));
var_dump(method_exists('ReflectionMethod', 'isDisabled'));
?>

<?php
function foo () {}

$function = new ReflectionFunction('sort');
var_dump($function->getExtension());

$function = new ReflectionFunction('foo');
var_dump($function->getExtension());
?>

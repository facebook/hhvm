<?php
$f = (new ReflectionFunction('iterator_to_array'))->getClosure();
$r = new ReflectionMethod($f, '__invoke');
var_dump($r->getParameters()[0]->getClass());
?>

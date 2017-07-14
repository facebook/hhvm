<?php

$c = (new ReflectionMethod('SplStack', 'count'))->getClosure(new SplStack);
$c = $c->bindTo(null);

$c = (new ReflectionFunction('strlen'))->getClosure();
$c = $c->bindTo(null);
var_dump($c("foo"));

?>

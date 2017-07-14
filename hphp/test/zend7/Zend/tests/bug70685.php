<?php

class cls {}

$c = (new ReflectionMethod('SplStack', 'count'))->getClosure(new SplStack);
$c = $c->bindTo(new cls);
var_dump($c);

$c = (new ReflectionMethod('SplStack', 'count'))->getClosure(new SplStack);
$c = $c->bindTo(new SplStack, 'cls');
var_dump($c);

?>

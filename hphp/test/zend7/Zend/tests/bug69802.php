<?php
$f = function(stdClass $x): stdClass {};
$r = new ReflectionMethod($f, '__invoke');
var_dump($r->getParameters()[0]->getName());
var_dump($r->getParameters()[0]->getClass());
echo $r->getParameters()[0], "\n";
echo $r->getReturnType()->getName(), "\n";
echo $r,"\n";
?>

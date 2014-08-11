<?php
class Test {
}
$c = new ReflectionClass('Test');
var_dump(new Test);
var_dump(new Test());
var_dump($c->newInstance());
var_dump($c->newInstanceArgs(array()));

try {
	var_dump($c->newInstanceArgs(array(1)));
} catch(ReflectionException $e) {
	echo $e->getMessage()."\n";
}
?>

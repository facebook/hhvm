<?hh

class foo { }

class_alias('foo', 'bar');

$a = new foo;
$b = new bar;

var_dump($a == $b, $a === $b);
var_dump($a instanceof $b);

var_dump($a is foo);
var_dump($a is bar);

var_dump($b is foo);
var_dump($b is bar);


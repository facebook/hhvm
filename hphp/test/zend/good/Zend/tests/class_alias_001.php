<?hh

class foo { }

class_alias('foo', 'bar');
<<__EntryPoint>> function main(): void {
$a = new foo;
$b = new bar;

var_dump($a == $b, $a === $b);

var_dump($a is foo);
var_dump($a is bar);

var_dump($b is foo);
var_dump($b is bar);
}

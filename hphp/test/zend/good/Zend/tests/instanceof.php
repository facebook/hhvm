<?hh
function __autoload($name) {
	echo("AUTOLOAD '$name'\n");
	eval("class $name {}");
}

class A {
}
$a = new A;
var_dump($a is B);
var_dump($a is A);

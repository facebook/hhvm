<?hh
<<__EntryPoint>> function main(): void {
$a = array("foo", "bar", "fubar");
$b = array("3" => "foo", "4" => "bar", "5" => "fubar");
$c = array("a" => "foo", "b" => "bar", "c" => "fubar");

/* simple array */
echo array_pop(inout $a), "\n";
array_push(inout $a, "foobar");
var_dump($a);

/* numerical assoc indices */
echo array_pop(inout $b), "\n";
var_dump($b);

/* assoc indices */
echo array_pop(inout $c), "\n";
var_dump($c);
}

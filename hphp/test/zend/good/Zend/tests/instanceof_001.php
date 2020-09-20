<?hh 
<<__EntryPoint>> function main(): void {
$a = new stdClass;
var_dump($a is stdClass);

var_dump(new stdCLass is stdClass);

$b = () ==> new stdClass;
var_dump($b() is stdClass);

$c = varray[new stdClass];
var_dump($c[0] is stdClass);

var_dump(@$inexistent is stdClass);

var_dump("$a" is stdClass);
}

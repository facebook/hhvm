<?hh

function foo($a) {
  print "foo $a\n";
}

function bar(&$a) {
  $a = 2;
  return $a;
}

function baz($a) {
  return 2;
}
<<__EntryPoint>> function main(): void {
$a = array(1, 2);
array_map(fun("foo"), $a);

$a = call_user_func(fun("baz"), $a);
var_dump($a);
}

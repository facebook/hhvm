<?hh

function foo($a) :mixed{
  print "foo $a\n";
}

function bar(inout $a) :mixed{
  $a = 2;
  return $a;
}

function baz($a) :mixed{
  return 2;
}
<<__EntryPoint>> function main(): void {
$a = vec[1, 2];
array_map(foo<>, $a);

$a = call_user_func(baz<>, $a);
var_dump($a);
}

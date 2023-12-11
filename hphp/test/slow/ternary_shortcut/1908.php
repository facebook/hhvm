<?hh

function foo() :mixed{
 var_dump('hello');
 return 789;
}

<<__EntryPoint>>
function main_1908() :mixed{
$a = 123 ?: 456;
var_dump($a);
$b = dict[123 => 456];
var_dump(isset($b[123]) ?: false);
var_dump(foo()?:123);
}

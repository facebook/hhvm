<?hh
const bar = 2, baz = bar + 1;

<<__EntryPoint>>
function foo() :mixed{
	$a = 1 + 1;
	$b = dict[bar => 1 + 1, baz * 2 => 1 << 2];
	$c = dict[1 => bar, 3 => baz];
	var_dump($a, $b, $c);
}

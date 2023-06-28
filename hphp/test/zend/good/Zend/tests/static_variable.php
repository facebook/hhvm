<?hh
const bar = 2, baz = bar + 1;

<<__EntryPoint>>
function foo() :mixed{
	$a = 1 + 1;
	$b = darray[bar => 1 + 1, baz * 2 => 1 << 2];
	$c = darray[1 => bar, 3 => baz];
	var_dump($a, $b, $c);
}

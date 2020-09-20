<?hh

function foo($p) { return $p; }

<<__EntryPoint>> function main(): void {
$a = darray(varray[1, 2, 3, 4]);
$a[123] = 5;
$a["0000"] = 6;
var_dump(foo(__LINE__));
var_dump(foo(varray[]));
var_dump(foo(varray[1, 2, 3]));
var_dump(foo($a[123]));
var_dump(foo($a[0000]));
try { var_dump(foo("$a[123]")); } catch (Exception $e) { echo $e->getMessage()."\n"; }
var_dump(foo("$a[0000]"));
}

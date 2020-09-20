<?hh
<<__EntryPoint>> function main(): void {
$a = <<<A
	A;
;
 A;
\;
A;

var_dump(strlen($a) == 12);
}

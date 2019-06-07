<?hh 
<<__EntryPoint>> function main() {
$a = <<<A
	A;
;
 A;
\;
A;

var_dump(strlen($a) == 12);
}

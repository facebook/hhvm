<?hh
<<__EntryPoint>> function main(): void {
$array = range(1, 10);

preg_grep('/asdf/', $array);

while (list($x) = each(inout $array)) {
	print $x;
}
}

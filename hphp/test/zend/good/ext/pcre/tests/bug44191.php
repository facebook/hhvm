<?hh
<<__EntryPoint>> function main(): void {
$array = range(1, 10);

preg_grep('/asdf/', $array);

foreach ($array as $x => $y) {
	print $x;
}
}

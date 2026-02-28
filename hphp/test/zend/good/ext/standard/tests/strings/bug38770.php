<?hh
<<__EntryPoint>> function main(): void {
foreach (vec['N','l'] as $v) {
	print_r(unpack($v, pack($v, -30000)));
}

echo "Done\n";
}

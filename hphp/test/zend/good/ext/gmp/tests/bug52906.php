<?hh
<<__EntryPoint>> function main(): void {
$vals = vec[
	vec[7, 3],
	vec[2, 7],
	vec[12, 7],
	vec[-2, 7],
	vec[-12, 7],
	vec[2, -7],
	vec[12, -7],
	vec[-2, -7],
	vec[-12, -7],
];
foreach($vals as $data) {
	echo "{$data[0]}%{$data[1]}=".gmp_strval(gmp_mod($data[0], $data[1]));
	echo "\n";
	echo "{$data[0]}%{$data[1]}=".gmp_strval(gmp_mod(gmp_init($data[0]), gmp_init($data[1])));
	echo "\n";
}
echo "Done\n";
}

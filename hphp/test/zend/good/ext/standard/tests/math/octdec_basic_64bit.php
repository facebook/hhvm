<?hh
<<__EntryPoint>> function main(): void {
echo "*** Testing octdec() : basic functionality ***\n";

$values = vec[01234567,
				0567,
				017777777777,
				020000000000,
				0x1234ABC,
				12345,
				'01234567',
				'0567',
				'017777777777',
				'020000000000',
				'0x1234ABC',
				'12345',
				31101.3,
				31.1013e5,				
				true,
				false,
				null];	

for ($i = 0; $i < count($values); $i++) {
	$res = octdec($values[$i]);
	var_dump($res);
}
echo "===Done===";
}

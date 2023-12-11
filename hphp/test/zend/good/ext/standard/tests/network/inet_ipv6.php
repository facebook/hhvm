<?hh
<<__EntryPoint>> function main(): void {
$a = vec[
	'::1',
	'::2',
	'::35',
	'::255',
	'::1024',
	'',
	'2001:0db8:85a3:08d3:1319:8a2e:0370:7344',
	'2001:0db8:1234:0000:0000:0000:0000:0000',
	'2001:0db8:1234:FFFF:FFFF:FFFF:FFFF:FFFF',
];

foreach ($a as $address) {
	$packed = inet_pton($address);
	var_dump(inet_ntop((string)$packed));
}

echo "Done\n";
}

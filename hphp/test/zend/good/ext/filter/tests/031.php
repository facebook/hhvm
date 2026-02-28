<?hh
<<__EntryPoint>> function main(): void {
$floats = vec[
'1.234   ',
'   1.234',
'1.234'	,
'1.2e3',
'7E3',
'7E3     ',
'  7E3     ',
'  7E-3     '
];

foreach ($floats as $float) {
	$out = filter_var($float, FILTER_VALIDATE_FLOAT);
	var_dump($out);
}

$floats = dict[
'1.234   '	=> ',',
'1,234'		=> ',',
'   1.234'	=> '.',
'1.234'		=> '..',
'1.2e3'		=> ','
];

echo "\ncustom decimal:\n";
foreach ($floats as $float => $dec) {
	$out = filter_var($float, FILTER_VALIDATE_FLOAT, dict["options"=>dict['decimal' => $dec]]);
	var_dump($out);
}
}

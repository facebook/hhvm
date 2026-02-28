<?hh
<<__EntryPoint>> function main(): void {
$numbers = vec[
	"0000000000000000", //0
	"2d431cebe2362a3f", //.0002
	"2e431cebe2362a3f", //.0002 + 10^-Accuracy[.0002]*1.01
	"0000000000001000", //2^-1022. (minimum normal double)
	"0100000000001000", //2^-1022. + 10^-Accuracy[2^-1022.]*1.01
	"ffffffffffffef7f", //2^1024. (maximum normal double)
	"feffffffffffef7f", //2^1024. - 10^-Accuracy[2^1024.]
	"0100000000000000", //minumum subnormal double
	"0200000000000000", //2nd minumum subnormal double
	"fffffffffffff000", //maximum subnormal double
	"fefffffffffff000", //2nd maximum subnormal double
	"0000000000000f7f", //+inf
	"0000000000000fff", //-inf
];

foreach ($numbers as $ns) {
  $num = unpack("d", pack("H*", $ns)); foreach ($num as $num) {}
	echo "number: ", sprintf("%.17e", $num), "... ";
	$num2 = unserialize(serialize($num));
  $repr = unpack("H*", pack("d", $num2)); foreach ($repr as $repr) {}
	if ($repr == $ns)
		echo "OK\n";
	else
		echo "mismatch\n\twas:    $ns\n\tbecame: $repr\n";
}
}

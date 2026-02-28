<?hh
<<__EntryPoint>> function main(): void {
$array = vec[
	"127.0.0.1",
	"10.0.0.1",
	"255.255.255.255",
	"255.255.255.0",
	"0.0.0.0",
	"66.163.161.116",
];

foreach ($array as $ip) {
	var_dump($long = ip2long($ip));
	var_dump(long2ip((string)$long));
}

try { var_dump(ip2long()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(ip2long(""));
var_dump(ip2long("777.777.777.777"));
var_dump(ip2long("111.111.111.111"));

try { var_dump(long2ip()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(long2ip('-110000'));
var_dump(long2ip(""));

echo "Done\n";
}

<?hh
<<__EntryPoint>> function main(): void {
$i = 75636;
$s1 = "this is a string";
$s2 = "876222numeric";
$s3 = "48474874";
$s4 = "25.68";

$c = $i + HH\Lib\Legacy_FIXME\cast_for_arithmetic($s1);
var_dump($c);

$c = $i + HH\Lib\Legacy_FIXME\cast_for_arithmetic($s2);
var_dump($c);

$c = $i + HH\Lib\Legacy_FIXME\cast_for_arithmetic($s3);
var_dump($c);

$c = $i + HH\Lib\Legacy_FIXME\cast_for_arithmetic($s4);
var_dump($c);

$c = HH\Lib\Legacy_FIXME\cast_for_arithmetic($s1) + $i;
var_dump($c);

$c = HH\Lib\Legacy_FIXME\cast_for_arithmetic($s2) + $i;
var_dump($c);

$c = HH\Lib\Legacy_FIXME\cast_for_arithmetic($s3) + $i;
var_dump($c);

$c = HH\Lib\Legacy_FIXME\cast_for_arithmetic($s4) + $i;
var_dump($c);

echo "Done\n";
}

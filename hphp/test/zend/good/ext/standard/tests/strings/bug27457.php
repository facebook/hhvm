<?hh <<__EntryPoint>> function main(): void {
$test = "Dot in brackets [.]\n";
echo $test;
$test = strtr($test, dict['.' => '0']);
echo $test;
$test = strtr($test, dict['0' => '.']);
echo $test;
$test = strtr($test, '.', '0');
echo $test;
$test = strtr($test, '0', '.');
echo $test;
}

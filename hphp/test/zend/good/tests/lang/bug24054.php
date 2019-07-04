<?hh
<<__EntryPoint>> function main(): void {
$long_max = is_int(5000000000)? 9223372036854775807 : 0x7FFFFFFF;
$long_min = -$long_max - 1;
printf("%d,%d,%d,%d\n",is_int($long_min  ),is_int($long_max  ),
                       is_int($long_min-1),is_int($long_max+1));

    $i = $long_max;

    $j = $i * 1001;
    $i *= 1001;

$tests = <<<TESTS
$i === $j
TESTS;

include(dirname(__FILE__) . '/../quicktester.inc');
}

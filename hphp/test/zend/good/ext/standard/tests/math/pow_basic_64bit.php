<?hh <<__EntryPoint>> function main(): void {
$bases = vec[23,
                -23,
                23.1,
                -23.1,
                2.345e1,
                -2.345e1,
                0x17,
                027,
                "23",
                "23.45",
                "2.345e1",
                PHP_INT_MAX,
                -PHP_INT_MAX - 1];

$exponents = vec[0,
               1,
               -1,
               2,
               -2,
               3,
               -3,
               2.5,
               -2.5,
               500,
               -500,
               2147483647,
               -2147483648];

foreach($bases as $base) {
    $base__str = (string)($base);
    echo "\n\nBase = $base__str";
    foreach($exponents as $exponent) {
        $exponent__str = (string)($exponent);
        echo "\n..... Exponent = $exponent__str Result = ";
        $res = pow(HH\Lib\Legacy_FIXME\cast_for_exponent($base), HH\Lib\Legacy_FIXME\cast_for_exponent($exponent));
        echo $res;
    }
    echo "\n\n";
}
echo "===Done===";
}

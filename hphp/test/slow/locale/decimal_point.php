<?hh


// PHP's string -> double conversion doesn't care about locale.
<<__EntryPoint>>
function main_decimal_point() :mixed{
setlocale(LC_ALL, 'fr_FR');

$a1 = '1.5';
$a2 = '1,5';
$b = 1;
var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic($a1) * $b);
var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic($a2) * $b);

var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic('1.5') * 1);
var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic('1,5') * 1);
}

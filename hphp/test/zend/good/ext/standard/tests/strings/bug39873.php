<?hh <<__EntryPoint>> function main(): void {
setlocale(LC_ALL, "ita","it","Italian","it_IT","it_IT.ISO8859-1","it_IT.ISO_8859-1");
$num = 0+HH\Lib\Legacy_FIXME\cast_for_arithmetic("1234.56");
echo number_format($num,2);
echo "\n";
}

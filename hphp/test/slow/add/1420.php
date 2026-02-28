<?hh


<<__EntryPoint>>
function main_1420() :mixed{
printf("%s\n", 30 + 30);
printf("%s\n", HH\Lib\Legacy_FIXME\cast_for_arithmetic("30") + 30);
printf("%s\n", 30 + HH\Lib\Legacy_FIXME\cast_for_arithmetic("30"));
printf("%s\n", HH\Lib\Legacy_FIXME\cast_for_arithmetic("30") + HH\Lib\Legacy_FIXME\cast_for_arithmetic("30"));
}

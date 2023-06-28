<?hh


<<__EntryPoint>>
function main_113() :mixed{
$foo = 1 + HH\Lib\Legacy_FIXME\cast_for_arithmetic("10.5");
$foo__str = (string)($foo);
print("$foo__str ");
$foo = 1 + HH\Lib\Legacy_FIXME\cast_for_arithmetic("-1.3e3");
$foo__str = (string)($foo);
print("$foo__str ");
$foo = 1 + HH\Lib\Legacy_FIXME\cast_for_arithmetic("bob-1.3e3");
print("$foo ");
$foo = 1 + HH\Lib\Legacy_FIXME\cast_for_arithmetic("bob3");
print("$foo ");
$foo = 1 + HH\Lib\Legacy_FIXME\cast_for_arithmetic("10 Small Pigs");
print("$foo ");
$foo = 4 + HH\Lib\Legacy_FIXME\cast_for_arithmetic("10.2 Little Piggies");
$foo__str = (string)($foo);
print("$foo__str ");
$foo = HH\Lib\Legacy_FIXME\cast_for_arithmetic("10.0 pigs ") + 1;
$foo__str = (string)($foo);
print("$foo__str ");
$foo = HH\Lib\Legacy_FIXME\cast_for_arithmetic("10.0 pigs ") + 1.0;
$foo__str = (string)($foo);
print("$foo__str ");
}

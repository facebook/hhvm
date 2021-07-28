<?hh

<<__EntryPoint>> function main(): void {
  var_dump(0 +  HH\Lib\Legacy_FIXME\cast_for_arithmetic("9223372036854775807"));
  var_dump(0 +  HH\Lib\Legacy_FIXME\cast_for_arithmetic("9223372036854775807 "));
  var_dump(0 + HH\Lib\Legacy_FIXME\cast_for_arithmetic("-9223372036854775808"));
  var_dump(0 + HH\Lib\Legacy_FIXME\cast_for_arithmetic("-9223372036854775808 "));
}

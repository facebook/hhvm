<?hh

function mult1($x) :mixed{
  return HH\Lib\Legacy_FIXME\cast_for_arithmetic($x) * 1;
}
<<__EntryPoint>> function main(): void {
var_dump(mult1(false));
var_dump(mult1(null));
var_dump(mult1(1));
var_dump(mult1(1.5));
var_dump(mult1(""));
var_dump(mult1("1"));
}

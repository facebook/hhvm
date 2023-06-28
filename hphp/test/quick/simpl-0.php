<?hh

function plus0($x) :mixed{
  return HH\Lib\Legacy_FIXME\cast_for_arithmetic($x) + 0;
}

function minus0($x) :mixed{
  return HH\Lib\Legacy_FIXME\cast_for_arithmetic($x) - 0;
}
<<__EntryPoint>> function main(): void {
var_dump(plus0(false));
var_dump(plus0(null));
var_dump(plus0(1));
var_dump(plus0(1.5));
var_dump(plus0(""));
var_dump(plus0("1"));

var_dump(minus0(false));
var_dump(minus0(null));
var_dump(minus0(1));
var_dump(minus0(1.5));
var_dump(minus0(""));
var_dump(minus0("1"));
}

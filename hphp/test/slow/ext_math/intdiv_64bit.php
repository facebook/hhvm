<?hh


// (int)(PHP_INT_MAX / 3) gives a different result
<<__EntryPoint>>
function main_intdiv_64bit() :mixed{
var_dump(intdiv(PHP_INT_MAX, 3));
}

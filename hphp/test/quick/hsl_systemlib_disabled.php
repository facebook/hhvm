<?hh

// The purpose of this test is to make sure that if the HSL systemlib is disabled
// the functions are undefined in HHVM

<<__EntryPoint>>
function hsl_systemlib_main(): void {
  var_dump(HH\Experimental\BuiltInLib\VecBI\map(vec[1, 2, 3], $x ==> $x * $x));
}

<?hh

// The purpose of this test is to make sure the HSL has been correctly
// embedded in HHVM, not to test the behavior of any particular function.

<<__EntryPoint>>
function hsl_systemlib_main(): void {
  var_dump(HH\Experimental\BuiltInLib\VecBI\map(vec[1, 2, 3], $x ==> $x * $x));
}

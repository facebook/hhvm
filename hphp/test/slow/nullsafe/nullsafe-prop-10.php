<?hh
function something($x) :mixed{
return NullsafeNullsafeProp10Php::$z;
}
function test() :mixed{

  $foo = null;
  $x = something($foo?->bar); // ok
  $x->n = 2;
  var_dump(NullsafeNullsafeProp10Php::$z);
}

abstract final class NullsafeNullsafeProp10Php {
  public static $z;
}
<<__EntryPoint>>
function entrypoint_nullsafeprop10(): void {

  NullsafeNullsafeProp10Php::$z = new stdClass;
  NullsafeNullsafeProp10Php::$z->m = 1;

  test();
}

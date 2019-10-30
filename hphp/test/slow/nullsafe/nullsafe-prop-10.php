<?hh // strict

NullsafeNullsafeProp10Php::$z = new stdClass;
NullsafeNullsafeProp10Php::$z->m = 1;
function something($x) {
return NullsafeNullsafeProp10Php::$z;
}
function test() {

  $foo = null;
  $x = something($foo?->bar); // ok
  $x->n = 2;
  var_dump(NullsafeNullsafeProp10Php::$z);
}

test();

abstract final class NullsafeNullsafeProp10Php {
  public static $z;
}

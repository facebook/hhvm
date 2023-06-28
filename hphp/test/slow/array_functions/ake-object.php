<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  protected $p;
}
class B extends A {
  public $p;
}

class C {
  private $priv;
}
class D extends C {
  private $priv;
  protected $prot;
  public $pub;
}

class E {
  public $unsetme;
}

class F {
  <<__LateInit>> public $p;
  public $q;
}

<<__EntryPoint>>
function main() :mixed{
  $o = new D();
  $o->dynamic = 1;
  echo "==== class D, extant ====\n";
  var_dump(array_key_exists("\0C\0priv", $o));
  var_dump(array_key_exists("\0D\0priv", $o));
  var_dump(array_key_exists("\0*\0prot", $o));
  var_dump(array_key_exists("pub", $o));
  var_dump(array_key_exists("dynamic", $o));
  echo "==== class D, bogus ====\n";
  var_dump(array_key_exists(1, $o));
  var_dump(array_key_exists('wat', $o));
  var_dump(array_key_exists("\0E\0priv", $o));
  var_dump(array_key_exists("\0*fish\0prot", $o));
  var_dump(array_key_exists("priv", $o));
  var_dump(array_key_exists("prot", $o));
$o = new stdClass();
//$o->{''} = 0;
$o->{'1'} = 'two';
$o->{'three'} = 'fore';
echo "==== stdClass, extant ====\n";
var_dump(array_key_exists('', $o));
var_dump(array_key_exists(1, $o));
var_dump(array_key_exists('three', $o));
echo "==== stdClass, bogus ====\n";
var_dump(array_key_exists(2, $o));
var_dump(array_key_exists("fife", $o));
var_dump(array_key_exists("\0stdClass\0three", $o));
var_dump(array_key_exists("\0*\0three", $o));

  $o = new A();
  echo "==== class A ====\n";
  var_dump(array_key_exists('p', $o));
  var_dump(array_key_exists("\0*\0p", $o));

  $o = new B();
  echo "==== class B ====\n";
  var_dump(array_key_exists('p', $o));
  var_dump(array_key_exists("\0*\0p", $o));

  $o = new E();
  echo "==== class E ====\n";
  var_dump(array_key_exists('unsetme', $o));
  unset($o->unsetme);
  var_dump(array_key_exists('unsetme', $o));

  $o = new F();
  echo "==== class F ====\n";
  var_dump(array_key_exists('p', $o));
  var_dump(array_key_exists('q', $o));
  $o->p = 1;
  var_dump(array_key_exists('p', $o));
  var_dump(array_key_exists('q', $o));

  $o = () ==> {};
  echo "==== Closure ====\n";
  var_dump(array_key_exists(0, $o));
  var_dump(array_key_exists(1, $o));
  var_dump(array_key_exists('', $o));

  $o = new DateTime();
  echo "==== DateTime, extant ====\n";
  var_dump(array_key_exists('date', $o));
  var_dump(array_key_exists('timezone_type', $o));
  var_dump(array_key_exists('timezone', $o));
  echo "==== DateTime, bogus ====\n";
  var_dump(array_key_exists(0, $o));
  var_dump(array_key_exists('nope', $o));

  $o = new SimpleXMLElement('<div color="red"><div/></div>');
  echo "==== SimpleXMLElement, extant ====\n";
  var_dump(array_key_exists('@attributes', $o));
  var_dump(array_key_exists('div', $o));
  echo "==== SimpleXMLElement, bogus ====\n";
  var_dump(array_key_exists(0, $o));
  var_dump(array_key_exists('a', $o));

  $o = Vector{1, 2, 3};
  echo "==== Vector, extant ====\n";
  var_dump(array_key_exists(0, $o));
  var_dump(array_key_exists(1, $o));
  var_dump(array_key_exists(2, $o));
  echo "==== Vector, bogus ====\n";
  var_dump(array_key_exists(3, $o));
  try {
    var_dump(array_key_exists('0', $o));
  } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try {
    var_dump(array_key_exists('', $o));
  } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try {
    var_dump(array_key_exists('a', $o));
  } catch (Exception $e) { echo $e->getMessage() . "\n"; }

  $o = Pair{2, 'a'};
  echo "==== Pair, extant ====\n";
  var_dump(array_key_exists(0, $o));
  var_dump(array_key_exists(1, $o));
  echo "==== Pair, bogus ====\n";
  var_dump(array_key_exists(2, $o));
  try {
    var_dump(array_key_exists('0', $o));
  } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try {
    var_dump(array_key_exists('', $o));
  } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try {
    var_dump(array_key_exists('a', $o));
  } catch (Exception $e) { echo $e->getMessage() . "\n"; }

  $o = Map{'a' => 'b', 3 => 4, '5' => 'six'};
  echo "==== Map, extant ====\n";
  var_dump(array_key_exists('a', $o));
  var_dump(array_key_exists(3, $o));
  var_dump(array_key_exists('5', $o));
  echo "==== Map, bogus ====\n";
  var_dump(array_key_exists(0, $o));
  var_dump(array_key_exists('3', $o));
  var_dump(array_key_exists(5, $o));
  var_dump(array_key_exists('b', $o));
}

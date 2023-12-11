<?hh

function test($thing) :mixed{
  echo "==== " . get_class($thing) . " may notice ====\n";

  echo "== read ==\n";
  $thing->dynprop = 3;
  $discard = $thing->dynprop;

  echo "== read (dynamic name) ==\n";
  $propname = __hhvm_intrinsics\launder_value("dyn") . "prop";
  $discard = $thing->$propname;

  echo "== setOp ==\n";
  $thing->dynprop += 2;

  echo "== incDec ==\n";
  $thing->dynprop++;

  echo "== dim for read ==\n";
  $thing->dynprop = dict['a' => 'b'];
  $discard = $thing->dynprop['a'];

  echo "== dim for read (quiet) ==\n";
  $discard = $thing->dynprop['z'] ?? 'w';
  $thing->dynprop = 3; // set prop back to an int

  echo "== get_object_vars ==\n";
  $discard = get_object_vars($thing);

  echo "== foreach ==\n";
  foreach ($thing as $k => $v) { }

  echo "== ReflectionProperty ==\n";
  $discard = new ReflectionProperty($thing, 'dynprop');

  // TODO: ReflectionClass constructor fatals on instances of
  // __PHP_Incomplete_Class
  if (!$thing is __PHP_Incomplete_Class) {
    echo "== ReflectionClass::getProperties ==\n";
    $rc = new ReflectionClass($thing);
    $discard = $rc->getProperties();
  }

  echo "== property_exists ==\n";
  $discard = property_exists($thing, 'dynprop');

  echo "== print_r ==\n";
  $discard = print_r($thing, true);

  echo "== var_export ==\n";
  $discard = var_export($thing, true);

  echo "== var_dump ==\n";
  var_dump($thing);

  echo "== debug_zval_dump ==\n";
  debug_zval_dump($thing);

  echo "== json_encode ==\n";
  $discard = json_encode($thing);

  echo "== serialize ==\n";
  $discard = serialize($thing);

  echo "== apc_store ==\n";
  apc_store('dynamic', $thing);

  echo "==== " . get_class($thing) . " never notice ====\n";

  echo "== dim for write ==\n";
  $thing->dynprop = dict['a' => 'b'];
  $thing->dynprop['c'] = 'd';
  $thing->dynprop = 3; // set prop back to an int




  echo "== clone ==\n";
  $discard = clone $thing;

  echo "== unset ==\n";
  unset($thing->dynprop);
}

function by_ref(inout $ref) :mixed{}

class C {}
class D { public $x = 1; }

<<__EntryPoint>>
function main() :mixed{
  test(new C());
  test(new D());
  test(gmp_init(0));
  test(new stdClass());
  test(unserialize('O:4:"Nope":0:{}')); // __PHP_Incomplete_Class
}

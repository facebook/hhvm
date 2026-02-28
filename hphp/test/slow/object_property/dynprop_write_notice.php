<?hh

function test($thing) :mixed{
  echo "==== " . get_class($thing) . " ====\n";
  $thing->dynprop = 3;
  $propname = __hhvm_intrinsics\launder_value("dynprop") . "2";
  $thing->$propname ??= 0;
  $thing->$propname += 2;
  error_boundary(inout $thing, (inout $o) ==> $o->dynprop3++);
  var_dump($thing);
  var_dump(unserialize(serialize($thing)));
  apc_store('dynamic', $thing);
  var_dump(__hhvm_intrinsics\apc_fetch_no_check('dynamic'));
}

class C {}

function error_boundary(inout $x, $fn) :mixed{
  try {
    $fn(inout $x);
  } catch (Exception $e) {
    print("Error: ".$e->getMessage()."\n");
  }
}

<<__EntryPoint>>
function main() :mixed{
  test(new C());
  test(gmp_init(0));
  test(new stdClass());
  test(unserialize('O:4:"Nope":0:{}')); // __PHP_Incomplete_Class
}

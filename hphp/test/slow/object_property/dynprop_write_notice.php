<?hh

function test($thing) {
  echo "==== " . get_class($thing) . " ====\n";
  $thing->dynprop = 3;
  $propname = __hhvm_intrinsics\launder_value("dynprop") . "2";
  $thing->$propname += 2;
  $thing->dynprop3++;
  var_dump($thing);
  var_dump(unserialize(serialize($thing)));
  apc_store('dynamic', $thing);
  var_dump(apc_fetch('dynamic'));
}

class C {}
class Extends_stdClass extends stdClass {}
class Extends___PHP_Incomplete_Class extends __PHP_Incomplete_Class {}

function main() {
  test(new C());
  test(gmp_init(0));
  test(new stdClass());
  test(unserialize('O:4:"Nope":0:{}')); // __PHP_Incomplete_Class
  test(new Extends_stdClass());
  test(new Extends___PHP_Incomplete_Class());
}

<<__EntryPoint>>
function main_dynprop_write_notice() {
main();
}

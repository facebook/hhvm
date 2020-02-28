<?hh

function test_empty(): void {
  echo "== empty array ==\n";
  var_dump(array_intersect_key(dict[1 => 2, '1' => '2'], varray[]));
  var_dump(array_intersect_key(varray[], dict[1 => 2, '1' => '2']));
}

function test_apc_local(): void {
  echo "== APC local array ==\n";
  var_dump(apc_store(__FILE__, darray[1 => 2, '1' => '2']));
  $a = __hhvm_intrinsics\apc_fetch_no_check(__FILE__);
  var_dump(array_intersect_key(dict[1 => 2, '1' => '2'], $a));
  var_dump(array_intersect_key($a, dict[1 => 2, '1' => '2']));
}

function test_globals(): void {
  echo "== GLOBALS array ==\n";
  $key = basename(__FILE__);
  $GLOBALS[$key] = 'lol';
  var_dump(array_intersect_key($GLOBALS['GLOBALS'], dict[$key => true]));
  var_dump(array_intersect_key(dict[$key => true], $GLOBALS['GLOBALS']));
}

<<__EntryPoint>>
function main(): void {
  test_empty();
  test_apc_local();
  test_globals();
}

<?hh

function test_empty(): void {
  echo "== empty array ==\n";
  var_dump(array_intersect_key(dict[1 => 2, '1' => '2'], array()));
  var_dump(array_intersect_key(array(), dict[1 => 2, '1' => '2']));
}

function test_apc_local(): void {
  echo "== APC local array ==\n";
  var_dump(apc_store(__FILE__, array(1 => 2, '1' => '2')));
  $a = apc_fetch(__FILE__);
  var_dump(array_intersect_key(dict[1 => 2, '1' => '2'], $a));
  var_dump(array_intersect_key($a, dict[1 => 2, '1' => '2']));
}

function test_globals(): void {
  echo "== GLOBALS array ==\n";
  $key = basename(__FILE__);
  $GLOBALS[$key] = 'lol';
  var_dump(array_intersect_key($GLOBALS, dict[$key => true]));
  var_dump(array_intersect_key(dict[$key => true], $GLOBALS));
}

<<__EntryPoint>>
function main(): void {
  test_empty();
  test_apc_local();
  test_globals();
}

<?hh

function test_empty(): void {
  echo "== empty array ==\n";
  var_dump(array_diff_key(dict[1 => 2, '1' => '2'], array()));
  var_dump(array_diff_key(array(), dict[1 => 2, '1' => '2']));
}

function test_apc_local(): void {
  echo "== APC local array ==\n";
  var_dump(apc_store(__FILE__, array(1 => 2, '1' => '2')));
  $a = apc_fetch(__FILE__);
  var_dump(array_diff_key(dict[1 => 2, '1' => '2'], $a));
  var_dump(array_diff_key($a, dict[1 => 2, '1' => '2']));
}

function test_globals(): void {
  echo "== GLOBALS array ==\n";

  $c = count($GLOBALS);
  $key = basename(__FILE__);
  $GLOBALS[$key] = 'lol';
  $c2 = count(array_diff_key($GLOBALS, dict[$key => true]));
  var_dump($c == $c2);

  var_dump(array_diff_key(dict[$key => true], $GLOBALS));
}

<<__EntryPoint>>
function main(): void {
  test_empty();
  test_apc_local();
  test_globals();
}

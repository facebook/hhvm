<?hh

function test_empty(): void {
  echo "== empty array ==\n";
  var_dump(array_diff_key(dict[1 => 2, '1' => '2'], vec[]));
  var_dump(array_diff_key(vec[], dict[1 => 2, '1' => '2']));
}

function test_apc_local(): void {
  echo "== APC local array ==\n";
  var_dump(apc_store(__FILE__, dict[1 => 2, '1' => '2']));
  $a = __hhvm_intrinsics\apc_fetch_no_check(__FILE__);
  var_dump(array_diff_key(dict[1 => 2, '1' => '2'], $a));
  var_dump(array_diff_key($a, dict[1 => 2, '1' => '2']));
}

<<__EntryPoint>>
function main(): void {
  test_empty();
  test_apc_local();
}

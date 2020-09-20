<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test($a, $k) {
  echo "=============== get ================================\n";
  try { var_dump($a[$k]); } catch (Exception $e) { echo $e->getMessage()."\n"; }
  echo "=============== isset===============================\n";
  var_dump(isset($a[$k]));
  echo "=============== akey-exists=========================\n";
  var_dump(array_key_exists($k, $a));
  echo "=============== idx ================================\n";
  var_dump(idx($a, $k));
  echo "=============== null-get ===========================\n";
  var_dump($a[$k] ?? 'def');
  echo "=============== set ================================\n";
  $a[$k] = 123;
  return $a;
}

function test_const_key($a) {
  echo "=============== get ================================\n";
  try { var_dump($a['2']); } catch (Exception $e) { echo $e->getMessage()."\n"; }
  echo "=============== isset===============================\n";
  var_dump(isset($a['2']));
  echo "=============== akey-exists=========================\n";
  var_dump(array_key_exists('2', $a));
  echo "=============== idx ================================\n";
  var_dump(idx($a, '2'));
  echo "=============== null-get ===========================\n";
  var_dump($a['2'] ?? 'def');
  echo "=============== set ================================\n";
  $a['2'] = 123;
  return $a;
}

function test_const_key_int($a) {
  echo "=============== get ================================\n";
  try { var_dump($a[2]); } catch (Exception $e) { echo $e->getMessage()."\n"; }
  echo "=============== isset===============================\n";
  var_dump(isset($a[2]));
  echo "=============== akey-exists=========================\n";
  var_dump(array_key_exists(2, $a));
  echo "=============== idx ================================\n";
  var_dump(idx($a, 2));
  echo "=============== null-get ===========================\n";
  var_dump($a[2] ?? 'def');
  echo "=============== set ================================\n";
  $a[2] = 123;
  return $a;
}

function test_casting($a) {
  echo "======================= dict cast ================\n";
  var_dump(dict($a));
  echo "======================= darray cast ================\n";
  var_dump(darray($a));
}

function run_tests() {
  $arr = $x ==> __hhvm_intrinsics\dummy_cast_to_kindofarray($x);

  echo "\n******* ad-hoc tests ********************************\n";
  test($arr(varray[]), '10');
  test($arr(varray[]), 10);
  test($arr(varray[1, 2, 3, 4]), '2');
  test(varray[1, 2, 3, 4], 2);
  test(darray[10 => 'abc'], '10');
  test(darray[10 => 'abc'], 10);

  echo "\n******* constant string tests ***********************\n";
  test_const_key($arr(varray[]));
  test_const_key($arr(varray[1, 2, 3, 4]));
  test_const_key(darray[2 => 'abc']);

  echo "\n******* constant int tests **************************\n";
  test_const_key_int($arr(varray[]));
  test_const_key_int(varray[1, 2, 3, 4]);
  test_const_key_int(darray[2 => 'abc']);

  echo "\n******* casting tests *******************************\n";
  test_casting(dict['1' => true, '2' => false]);
  test_casting(keyset['1', '2', '3']);

  echo "======================= literals ===================\n";
  var_dump(darray(dict['1' => true, '2' => false]));
  var_dump(darray(keyset['1', '2', '3']));
}

<<__EntryPoint>>
function main_intlike() {
run_tests();
}

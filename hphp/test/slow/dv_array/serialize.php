<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function serialize_test($a) :mixed{
  var_dump(
    __hhvm_intrinsics\serialize_keep_dvarrays(
      __hhvm_intrinsics\launder_value($a)
    )
  );
  var_dump(serialize(__hhvm_intrinsics\launder_value($a)));
  echo "====================================================\n";
}

function unserialize_test($s) :mixed{
  echo "unserialize_test(\"".$s."\")\n";
  $a = unserialize(__hhvm_intrinsics\launder_value($s));
  var_dump($a);
  var_dump(is_varray($a));
  var_dump(is_darray($a));
  echo "----------------------------------------------------\n";
  $a = unserialize(
    __hhvm_intrinsics\launder_value($s),
    darray['force_darrays' => true]
  );
  var_dump($a);
  var_dump(is_varray($a));
  var_dump(is_darray($a));
  echo "====================================================\n";
}

function round_trip($a) :mixed{
  $a2 = __hhvm_intrinsics\launder_value($a);
  $a3 = unserialize(
    __hhvm_intrinsics\serialize_keep_dvarrays($a2)
  );
  if ($a2 !== $a3) {
    echo "============ Value mismatch ================\n";
    var_dump($a2);
    var_dump($a3);
  } else if (is_varray($a2) !== is_varray($a3)) {
    echo "============ is_varray mismatch ============\n";
    var_dump($a2);
    var_dump($a3);
    var_dump(is_varray($a2));
    var_dump(is_varray($a3));
  } else if (is_darray($a2) !== is_darray($a3)) {
    echo "============ is_darray mismatch ============\n";
    var_dump($a2);
    var_dump($a3);
    var_dump(is_darray($a2));
    var_dump(is_darray($a3));
  }
}

function serialize_tests() :mixed{
  serialize_test(varray[]);
  serialize_test(darray[100 => 200, 200 => 300, 300 => 400]);
  serialize_test(darray[0 => 'a', 1 => 'b', 2 => 'c']);
  serialize_test(darray['abc' => 100, 'def' => 200, 'ghi' => 300]);
  serialize_test(darray[
    1 => darray[2 => 3],
    4 => darray[5 => 6],
  ]);

  serialize_test(varray[]);
  serialize_test(varray[123, 456, 789]);
  serialize_test(varray['abc', 'def', 'ghi']);
  serialize_test(varray[varray[1, 2, 3], varray[4, 5, 6]]);
  serialize_test(varray[varray[1, 2, 3], varray[4, 5, 6]]);

  serialize_test(darray[]);
  serialize_test(darray[100 => 200, 200 => 300, 300 => 400]);
  serialize_test(darray[0 => 'a', 1 => 'b', 2 => 'c']);
  serialize_test(darray['abc' => 100, 'def' => 200, 'ghi' => 300]);
  serialize_test(darray[1 => darray[2 => 3], 4 => darray[5 => 6]]);
  serialize_test(darray[1 => darray[2 => 3], 4 => darray[5 => 6]]);

  serialize_test(varray[darray[0 => 'a'], darray[1 => 'b'], darray[2 => 'c']]);
  serialize_test(darray[0 => varray[1, 2, 3], 1 => varray[4, 5, 6]]);
}

function unserialize_tests() :mixed{
  unserialize_test('a:0:{}');
  unserialize_test('a:3:{i:100;i:123;i:200;i:456;i:300;i:789;}');
  unserialize_test('a:3:{i:100;s:3:"abc";i:200;s:3:"def";i:300;s:3:"ghi";}');
  unserialize_test('a:3:{i:0;s:3:"abc";i:1;s:3:"def";i:2;s:3:"ghi";}');
  unserialize_test('a:3:{s:3:"abc";i:100;s:3:"def";i:200;s:3:"ghi";i:300;}');

  unserialize_test('y:0:{}');
  unserialize_test('y:3:{i:123;i:456;i:789;}');
  unserialize_test('y:3:{s:3:"abc";s:3:"def";s:3:"ghi";}');
  unserialize_test('y:2:{y:3:{i:1;i:2;i:3;}y:3:{i:4;i:5;i:6;}}');
  unserialize_test('a:2:{i:0;y:3:{i:1;i:2;i:3;}i:1;y:3:{i:4;i:5;i:6;}}');

  unserialize_test('Y:0:{}');
  unserialize_test('Y:3:{i:100;i:123;i:200;i:456;i:300;i:789;}');
  unserialize_test('Y:3:{i:100;s:3:"abc";i:200;s:3:"def";i:300;s:3:"ghi";}');
  unserialize_test('Y:3:{i:0;s:3:"abc";i:1;s:3:"def";i:2;s:3:"ghi";}');
  unserialize_test('Y:3:{s:3:"abc";i:100;s:3:"def";i:200;s:3:"ghi";i:300;}');

  unserialize_test('Y:2:{i:1;Y:1:{i:2;i:3;}i:4;Y:1:{i:5;i:6;}}');
  unserialize_test('a:2:{i:1;Y:1:{i:2;i:3;}i:4;Y:1:{i:5;i:6;}}');
  unserialize_test('y:3:{Y:1:{i:0;s:1:"a";}Y:1:{i:1;s:1:"b";}Y:1:{i:2;s:1:"c";}}');
  unserialize_test('Y:2:{i:0;y:3:{i:1;i:2;i:3;}i:1;y:3:{i:4;i:5;i:6;}}');
}

function round_trip_tests() :mixed{
  round_trip(varray[]);
  round_trip(varray[123, 456, 789]);
  round_trip(varray['abc', 'def', 'ghi']);
  round_trip(varray[varray[1, 2, 3], varray[4, 5, 6]]);

  round_trip(darray[]);
  round_trip(darray[100 => 200, 200 => 300, 300 => 400]);
  round_trip(darray[0 => 'a', 1 => 'b', 2 => 'c']);
  round_trip(darray['abc' => 100, 'def' => 200, 'ghi' => 300]);
  round_trip(darray[1 => darray[2 => 3], 4 => darray[5 => 6]]);

  round_trip(varray[darray[0 => 'a'], darray[1 => 'b'], darray[2 => 'c']]);
  round_trip(darray[0 => varray[1, 2, 3], 1 => varray[4, 5, 6]]);
}


<<__EntryPoint>>
function main_serialize() :mixed{
serialize_tests();
unserialize_tests();
round_trip_tests();
}

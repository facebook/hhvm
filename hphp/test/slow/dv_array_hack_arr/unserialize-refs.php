<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test($s) {
  $a = __hhvm_intrinsics\deserialize_keep_dvarrays(
    __hhvm_intrinsics\launder_value($s)
  );
  var_dump($a);
}


<<__EntryPoint>>
function main_unserialize_refs() {
test('y:3:{y:1:{i:987;}i:456;R:2;}');
test('Y:2:{i:1;i:123;i:2;R:2;}');
}

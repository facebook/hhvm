<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function main() {
  var_dump(unserialize('y:0:{}'));
  var_dump(unserialize('y:3:{i:123;i:456;i:789;}'));
  var_dump(unserialize('y:3:{s:3:"abc";s:3:"def";s:3:"ghi";}'));

  var_dump(unserialize('Y:0:{}'));
  var_dump(unserialize('Y:3:{i:100;i:123;i:200;i:456;i:300;i:789;}'));
  var_dump(unserialize('Y:3:{i:100;s:3:"abc";i:200;s:3:"def";i:300;s:3:"ghi";}'));
  var_dump(unserialize('Y:3:{s:3:"abc";i:100;s:3:"def";i:200;s:3:"ghi";i:300;}'));
}
main();

<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function main($a) {
  $v = varray[&$a];
  var_dump($v);
}
main(100);

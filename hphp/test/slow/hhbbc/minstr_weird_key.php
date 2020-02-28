<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C {}

function elem() {
  $k = new C();

  $b = varray[4, 3];
  @var_dump($b[$k] = 4);
}


<<__EntryPoint>>
function main_minstr_weird_key() {
elem();
}

<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function main() {
  $stuffs = vec[
    array(42, 42),
    varray[42, 42],
    tuple(42, 42),
    vec[42, 42],
    array('forty-two' => 42),
    darray['forty-two' => 42],
    shape('forty-two' => 42),
    dict['forty-two' => 42],
  ];

  foreach ($stuffs as $dude) {
    var_dump($dude is (int, int));
    var_dump($dude is shape('forty-two' => int));
  }
}

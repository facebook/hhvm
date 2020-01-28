<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function do_add($a, $b) {
  var_dump($a + $b);
}

function main() {
  do_add([], []);
  do_add(varray[1, 2, 3], []);
  do_add([], varray[1, 2, 3]);
  do_add(varray[1, 2, 3], varray[4, 5, 6]);
}

<<__EntryPoint>>
function main_add() {
main();
}

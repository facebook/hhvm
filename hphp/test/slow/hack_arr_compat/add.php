<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function do_add($a, $b) :mixed{
  try { var_dump($a + $b); } catch (Exception $e) { print($e->getMessage()."\n"); }
}

function main() :mixed{
  do_add(dict[], dict[]);
  do_add(darray(vec[1, 2, 3]), dict[]);
  do_add(dict[], darray(vec[1, 2, 3]));
  do_add(darray(vec[1, 2, 3]), darray(vec[4, 5, 6]));
}

<<__EntryPoint>>
function main_add() :mixed{
main();
}

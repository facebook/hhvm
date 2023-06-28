<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function do_print($f, $v, $k, $d) :mixed{
  $f($v); print "\n";
  $f($k); print "\n";
  $f($d); print "\n";
}

function run($v, $k, $d) :mixed{
  do_print(var_dump<>, $v, $k, $d);
  do_print(var_export<>, $v, $k, $d);
  do_print(print_r<>, $v, $k, $d);
}
<<__EntryPoint>> function main(): void {
run(vec['a', 'b', 'c'],
    keyset['a', 'b', 'c'],
    dict[0 => 'a', 1 => 'b', 2 => 'c']);
run(vec[], keyset[], dict[]);
}

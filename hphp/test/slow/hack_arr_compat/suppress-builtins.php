<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test_array_diff1() { return array_diff(varray[1, '2'], varray['1']); }
function test_array_diff2($a, $b) { return array_diff($a, $b); }

function test_array_column1() {
  return array_column(
    varray[darray[2 => '3', 4 => '5'], darray[2 => '6', 4 => '5']],
    '2',
    '4'
  );
}
function test_array_column2($a, $b, $c) { return array_column($a, $b, $c); }

function test_array_count_values1() { return array_count_values(varray['5', 5]); }
function test_array_count_values2($a) { return array_count_values($a); }

function test_array_intersect1() { return array_intersect(varray['5'], varray[5]); }
function test_array_intersect2($a, $b) { return array_intersect($a, $b); }

function test() {
  var_dump(test_array_diff1());
  var_dump(test_array_column1());
  var_dump(test_array_count_values1());
  var_dump(test_array_intersect1());

  var_dump(
    test_array_diff2(
      __hhvm_intrinsics\launder_value(varray[1, '2']),
      __hhvm_intrinsics\launder_value(varray['1'])
    )
  );
  var_dump(
    test_array_column2(
      __hhvm_intrinsics\launder_value(
        varray[darray[2 => '3', 4 => '5'], darray[2 => '6', 4 => '5']]
      ),
      __hhvm_intrinsics\launder_value('2'),
      __hhvm_intrinsics\launder_value('4')
    )
  );
  var_dump(
    test_array_count_values2(
      __hhvm_intrinsics\launder_value(varray['5', 5])
    )
  );
  var_dump(
    test_array_intersect2(
      __hhvm_intrinsics\launder_value(varray['5']),
      __hhvm_intrinsics\launder_value(varray[5])
    )
  );
}

<<__EntryPoint>>
function main_suppress_builtins() {
test();
}

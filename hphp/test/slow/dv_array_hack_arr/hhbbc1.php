<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function launder($x) { return __hhvm_intrinsics\launder_value($x); }

function get() {
  if (launder(true)) {
    return darray[
      launder('a') => launder(100),
      launder('b') => launder(200),
      launder('c') => launder(300)
    ];
  } else {
    return darray[];
  }
}

function foo() {
  $x = get();
  if ($x === __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])) return varray[];
  return varray['a', 'b', 'c'];
}

function test() {
  $x = foo();
  var_dump($x);
  var_dump(is_varray($x));
  var_dump(is_darray($x));
}

<<__EntryPoint>>
function main_hhbbc1() {
test();
}

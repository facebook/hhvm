<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function launder($x) :mixed{ return __hhvm_intrinsics\launder_value($x); }

function get() :mixed{
  if (launder(true)) {
    return dict[
      launder('a') => launder(100),
      launder('b') => launder(200),
      launder('c') => launder(300)
    ];
  } else {
    return dict[];
  }
}

function foo() :mixed{
  $x = get();
  if ($x === dict[]) return vec[];
  return vec['a', 'b', 'c'];
}

function test() :mixed{
  $x = foo();
  var_dump($x);
  var_dump(is_varray($x));
  var_dump(is_darray($x));
}

<<__EntryPoint>>
function main_hhbbc1() :mixed{
test();
}

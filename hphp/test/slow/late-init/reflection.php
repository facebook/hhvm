<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  <<__LateInit>> public $p1;
  <<__LateInit>> public static $p2;
}

function test() :mixed{
  $a = new A();
  var_dump(hphp_get_property($a, '', 'p1'));
  var_dump(hphp_get_static_property('A', 'p2', false));

  $a->p1 = 700;
  A::$p2 = 800;
  var_dump(hphp_get_property($a, '', 'p1'));
  var_dump(hphp_get_static_property('A', 'p2', false));
}

<<__EntryPoint>>
function main_reflection() :mixed{
test();
}

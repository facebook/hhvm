<?hh

function id($a) :mixed{
 return $a;
}
class X {
}
class Y extends X {
 function t() :mixed{}
}
function test() :mixed{
  id(new Y)->t();
}

<<__EntryPoint>>
function main_1836() :mixed{
  if (__hhvm_intrinsics\launder_value(0)) {
    include '1836.inc';
  }
}

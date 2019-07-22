<?hh

function id($a) {
 return $a;
}
class X {
}
class Y extends X {
 function t() {}
}
function test() {
  id(new Y)->t();
}

<<__EntryPoint>>
function main_1836() {
  if (__hhvm_intrinsics\launder_value(0)) {
    include '1836.inc';
  }
}

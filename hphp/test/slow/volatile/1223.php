<?hh

class B {
}
class A extends B {
  static function make() {
    $b = new parent();
    $a = new self();
  }
}

<<__EntryPoint>>
function main_1223() {
  if (__hhvm_intrinsics\launder_value(false)) {
    include '1223.inc';
  }
  A::make();
}

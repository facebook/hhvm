<?hh

class b {
  function z() :mixed{
    $this->x();
  }
  function y() :mixed{
    echo 'y';
  }
}
class c extends b {
  function x() :mixed{
    $this->y();
  }
}

<<__EntryPoint>>
function main_1481() :mixed{
  if (__hhvm_intrinsics\launder_value(false)) {
    include '1481.inc';
  }
  $x = new c();
  $x->z();
}

<?hh

class a {
  public static function x() :mixed{
    echo 'x';
  }
}
class b extends a{
  public static function z() :mixed{
    self::x();
  }
}

<<__EntryPoint>>
function main_1466() :mixed{
  if (__hhvm_intrinsics\launder_value(0)) {
    include '1466-1.inc';
  }
  b::x();
}

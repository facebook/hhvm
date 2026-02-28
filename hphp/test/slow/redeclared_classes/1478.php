<?hh

class A {
  static function bar(inout $a) :mixed{
    $a = 'ok';
  }
}
class C {
  static function bar(inout $a) :mixed{
  }
}
class A2 extends C {
  static function bar(inout $a) :mixed{
    $a = 'ok';
  }
}

<<__EntryPoint>>
function main_1478() :mixed{
  $a = 'failed';
  A::bar(inout $a);
  var_dump($a);
  if (__hhvm_intrinsics\launder_value(false)) {
    include '1478.inc';
  }
  $a = 'failed';
  A2::bar(inout $a);
  var_dump($a);
}

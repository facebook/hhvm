<?hh

class A {
  function t($a, $arr) :mixed{
  }
}
class B extends A {
  function t($a, $arr) :mixed{
    var_dump($a);
    $arr['hello'] = $a;
    var_dump($a);
  }
}
function f() :mixed{
  return new B();
}
function test() :mixed{
  $v = 100;
  $arr = dict['hello' => $v];
  $a = new B();
  $a->t($arr['hello'], $arr);
}
function test2(inout $a, $b) :mixed{
  $a = $b;
}

<<__EntryPoint>>
function main_1090() :mixed{
  test();
  $v = 10;
  test2(inout $v, $v);
  var_dump($v);
}

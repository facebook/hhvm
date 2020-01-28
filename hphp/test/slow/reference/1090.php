<?hh

class A {
  function t($a, $arr) {
  }
}
class B extends A {
  function t($a, $arr) {
    var_dump($a);
    $arr['hello'] = $a;
    var_dump($a);
  }
}
function f() {
  return new B();
}
function test() {
  $v = 100;
  $arr = darray['hello' => $v];
  $a = new B();
  $a->t($arr['hello'], $arr);
}
function test2(inout $a, $b) {
  $a = $b;
}

<<__EntryPoint>>
function main_1090() {
  test();
  $v = 10;
  test2(inout $v, $v);
  var_dump($v);
}

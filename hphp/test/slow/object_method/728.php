<?hh

class A1 {
  function a1f($a) {
    var_dump('a1f:0');
  }
  static function a1b($a) {
    var_dump('a1b:0');
  }
}
class B1 extends A1 {
  function b1f($a) {
    var_dump('b1f:0');
  }
  static function b1b($a) {
    var_dump('b1b:0');
  }
}

<<__EntryPoint>>
function main_728() {
  $a1 = new A1();
  $b1=  new B1();
  $f = 'a1f';
  $b = 'a1b';
  $a1->$f(1);
  A1::$b(1);
  $b1->$f(1);
  B1::$b(1);
  $f = 'b1f';
  $b = 'b1b';
  $b1->$f(1);
  B1::$b(1);
  $f = 'b2f';
  $b = 'b2b';
  call_user_func(varray[$b1, 'b1f'], 1);
}

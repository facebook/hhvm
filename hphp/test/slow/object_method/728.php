<?hh

class A1 {
  <<__DynamicallyCallable>> function a1f($a) :mixed{
    var_dump('a1f:0');
  }
  <<__DynamicallyCallable>> static function a1b($a) :mixed{
    var_dump('a1b:0');
  }
}
class B1 extends A1 {
  <<__DynamicallyCallable>> function b1f($a) :mixed{
    var_dump('b1f:0');
  }
  <<__DynamicallyCallable>> static function b1b($a) :mixed{
    var_dump('b1b:0');
  }
}

<<__EntryPoint>>
function main_728() :mixed{
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
  call_user_func(vec[$b1, 'b1f'], 1);
}

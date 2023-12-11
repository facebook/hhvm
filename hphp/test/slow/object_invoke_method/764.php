<?hh

// standard execution
class C1 {
  public static function __invoke($a0, $a1) :mixed{
    var_dump('C1');
    var_dump($a0, $a1);
  }
}
class D1 extends C1 {
}
class E1 extends D1 {
  public static function __invoke($a0, $a1) :mixed{
    var_dump('D2');
    var_dump($a0, $a1);
  }
  public function test() :mixed{
    C1::__invoke(0, 1);
    D1::__invoke(0, 1);
    E1::__invoke(0, 1);
    call_user_func(mk('C1'), 0, 1);
    call_user_func(mk('D1'), 0, 1);
    call_user_func(mk('E1'), 0, 1);
  }
}
class F1 {
  public function __invoke($a0) :mixed{
    return $a0 > 10;
  }
}
function mk($n) :mixed{
  return $n . '::__invoke';
}

<<__EntryPoint>>
function main_764() :mixed{
$c = new C1;
$d = new D1;
$e = new E1;
$c(0, 1);
$d(0, 1);
$e(0, 1);
call_user_func($c, 0, 1);
call_user_func($d, 0, 1);
call_user_func($e, 0, 1);
call_user_func_array($c, vec[0, 1]);
call_user_func_array($d, vec[0, 1]);
call_user_func_array($e, vec[0, 1]);
$c::__invoke(0, 1);
$d::__invoke(0, 1);
$e::__invoke(0, 1);
(new E1)->test();
var_dump(array_filter(vec[0, 1, 11, 13], new F1));
}

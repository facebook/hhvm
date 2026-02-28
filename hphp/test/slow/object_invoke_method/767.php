<?hh

// as a static method
class C4 {
  public static function __invoke($a0, $a1) :mixed{
    var_dump('C4');
    var_dump($a0, $a1);
  }
}
class D4 extends C4 {
}
class E4 extends D4 {

  <<__LSB>>
  private static $__invokeX = 0;
  public static function __invoke($a0, $a1) :mixed{
    var_dump('E4');
    var_dump($a0, $a1);
    var_dump(static::$__invokeX ++);
  }
}
class C5 {

  <<__LSB>>
  private static $__invokeX = 0;
  public static function __invoke() :mixed{
    var_dump(static::$__invokeX ++);
  }
}
class D5 extends C5 {
}

<<__EntryPoint>>
function main_767() :mixed{
$c = new C4;
$d = new D4;
$c(0, 1);
$d(0, 1);
call_user_func($c, 0, 1);
call_user_func($d, 0, 1);
C4::__invoke(0, 1);
D4::__invoke(0, 1);
$e = new E4;
$e(0, 1);
$e(0, 1);
call_user_func($e, 0, 1);
E4::__invoke(0, 1);
$c = new C5;
$d = new D5;
$c();
 $d();
$c();
 $d();
}

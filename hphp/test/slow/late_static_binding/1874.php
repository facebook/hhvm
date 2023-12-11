<?hh

class B {
  public static $a = 100;
  <<__DynamicallyCallable>> static function f() :mixed{
    var_dump(static::$a);
  }
}
class C extends B {
  public static $a = 1000;
}

<<__EntryPoint>>
function main_1874() :mixed{
call_user_func(vec['C', 'f']);
}

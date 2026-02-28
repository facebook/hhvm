<?hh

function wrapper($a) :mixed{
  if ($a) {
    include '1227.inc';
  }
}
class C2 {
  private static $v;
  public static function f() :mixed{
    return self::$v;
  }
}
function foo($a) :mixed{
  if ($a == 0) return is_callable(vec['C', 'f'], false);
  return is_callable(vec['C2', 'f'], false);
}

<<__EntryPoint>>
function main_1227() :mixed{
wrapper(false);
var_dump(foo(0));
var_dump(foo(1));
if (class_exists('C')) var_dump('yes');
 else var_dump('no');
}

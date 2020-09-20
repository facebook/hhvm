<?hh

function wrapper($a) {
  if ($a) {
    include '1227.inc';
  }
}
class C2 {
  private static $v;
  public static function f() {
    return self::$v;
  }
}
function foo($a) {
  if ($a == 0) return is_callable(varray['C', 'f'], false);
  return is_callable(varray['C2', 'f'], false);
}

<<__EntryPoint>>
function main_1227() {
wrapper(false);
var_dump(foo(0));
var_dump(foo(1));
if (class_exists('C')) var_dump('yes');
 else var_dump('no');
}

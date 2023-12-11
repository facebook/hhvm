<?hh
class c {
  const A = 'a';
  const B = 'b';
  const C = 'c';
  const D = 'd';
  public static $S = vec[    self::A,    self::B,    self::C,    self::D];
}


<<__EntryPoint>>
function main_1222() :mixed{
class_exists('c');
var_dump(c::$S);
}

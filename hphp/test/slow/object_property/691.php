<?hh
class X {
  const UNKNOWN = 1;
  public $foo = -1;
  static public $bar = FOO;
  public $baz = self::UNKNOWN;
}


const FOO = 'foo';
<<__EntryPoint>>
function main_691() {
$vars = get_class_vars('X');
 asort(inout $vars);
 var_dump($vars);
}

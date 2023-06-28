<?hh
class X {
  const UNKNOWN = 1;
  public $foo = -1;
  static public $bar = FOO;
  public $baz = self::UNKNOWN;
}


const FOO = 'foo';
<<__EntryPoint>>
function main_691() :mixed{
$vars = get_class_vars('X');
 uasort(inout $vars,  HH\Lib\Legacy_FIXME\cmp<>);
 var_dump($vars);
}

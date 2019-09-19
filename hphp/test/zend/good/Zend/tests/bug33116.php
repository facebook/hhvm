<?hh
function __autoload($class)
{
  $GLOBALS['include'] ??= [];
  $GLOBALS['include'][] = $class;
  eval("class DefClass{}");
}
<<__EntryPoint>> function main(): void {
$a = new DefClass;
print_r($a);
print_r($GLOBALS['include']);
}

<?hh

class foo{
  public function __get($n)  {
 return 'foo';
 }
  public function __set($n,$v)  {
 }
}

<<__EntryPoint>>
function main_759() {
$foo = new foo;
 $a = $foo->x = 'baz';
 $b = $foo->x .= 'bar';
var_dump($a,$b);
}

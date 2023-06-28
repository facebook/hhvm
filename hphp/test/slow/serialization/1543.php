<?hh

class b {
  private $foo = 1;
  private $bar = 2;
}
class b2 extends b {
  public $bar = 3;
}

<<__EntryPoint>>
function main_1543() :mixed{
$x = new b2;
$x->foo = 100;
var_dump(serialize($x));
var_dump($x);
}

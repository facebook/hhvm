<?hh

class A {
  public $foo = 0;
  private $foopriv = 2;
  function __clone() :mixed{
    echo "clone
";
  }
}

<<__EntryPoint>>
function main_1497() :mixed{
$a1 = new A;
$a1->foo = 'foo';
$a1->dyn = 'dyn';
var_dump($a1);
$a2 = clone $a1;
var_dump($a1);
var_dump($a2);
$a2->foo = 'a2foo';
$a2->dyn = 'a2dyn';
var_dump($a1);
var_dump($a2);
}

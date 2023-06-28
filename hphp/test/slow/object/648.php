<?hh

class A {
  public $a = 2;
}
class B {
  public $b = 3;
}

<<__EntryPoint>>
function main_648() :mixed{
$obj = new A();
 var_dump($obj);
 var_dump($obj->a);
$obj = new B();
 var_dump($obj);
 var_dump($obj->b);
}

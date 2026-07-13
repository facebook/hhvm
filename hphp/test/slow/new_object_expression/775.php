<?hh

class A {
  public $num;
}

<<__EntryPoint>>
function main() :mixed{
  $a=new A(); foreach($a as $v);
  $a->num = 1;
  print($a->num);
}

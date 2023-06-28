<?hh

class A {
  public $num;
}

<<__EntryPoint>>
function main() :mixed{
  foreach(($a=new A()) as $v);
  $a->num = 1;
  print($a->num);
}

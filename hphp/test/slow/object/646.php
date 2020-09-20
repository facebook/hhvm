<?hh
class A {
  public $b = 3;
  public $a = 2;
}
<<__EntryPoint>> function main(): void {
$obj = new A();
 var_dump($obj);
 var_dump($obj->c);
}

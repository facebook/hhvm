<?hh

class A {
  public function t() :mixed{
    var_dump(Object654::$a);
  }
}

abstract final class Object654 {
  public static $a;
}
<<__EntryPoint>> function main(): void {
Object654::$a = 1;
$obj = new A();
$obj->t();
}

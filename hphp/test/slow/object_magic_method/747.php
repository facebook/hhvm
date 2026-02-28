<?hh

class A {
  private $foo, $bar;
   function __construct() {
 $this->foo = 1;
 $this->bar = 2;
}
   public function __sleep() :mixed{
 $this->foo = 3;
 return vec['foo'];
}
 }

 <<__EntryPoint>>
function main_747() :mixed{
$a = new A();
 var_dump(serialize($a));
}

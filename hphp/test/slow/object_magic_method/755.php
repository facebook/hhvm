<?hh

class C {
  public $foo = 123;
  public function __unset($k) {
 echo "__unset $k\n";
 }
}

<<__EntryPoint>>
function main_755() {
$obj = new C;
unset($obj->foo);
}

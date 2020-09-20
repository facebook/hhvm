<?hh

class A {
 static $a = 10;
 public $b = 20;
}

<<__EntryPoint>>
function main_1337() {
$obj = new A();
 var_dump(get_object_vars($obj));
}

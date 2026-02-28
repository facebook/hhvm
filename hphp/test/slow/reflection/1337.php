<?hh

class A {
 public static $a = 10;
 public $b = 20;
}

<<__EntryPoint>>
function main_1337() :mixed{
$obj = new A();
 var_dump(get_object_vars($obj));
}

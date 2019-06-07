<?hh

class A implements Countable {
 public function count() {
 return 1;
}
}

 <<__EntryPoint>>
function main_515() {
$obj = new A();
 var_dump(count($obj));
}

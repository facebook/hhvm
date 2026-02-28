<?hh

class A implements Countable {
 public function count() :mixed{
 return 1;
}
}

 <<__EntryPoint>>
function main_515() :mixed{
$obj = new A();
 var_dump(count($obj));
}

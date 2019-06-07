<?hh

class Foo {
  public function __get($prop) { echo "__get($prop)\n"; }
  public function __set($prop, $val) { echo "__set($prop, $val)\n"; }
  public function __isset($prop) { echo "__isset($prop)\n"; }
}


<<__EntryPoint>>
function main_null_prop_magic_miss() {
$obj = new Foo;
$prop = "\0myprop";

$obj->$prop = "should work";
$_ = $obj->$prop;
isset($obj->$prop);
unset($obj->$prop);
}

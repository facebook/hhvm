<?hh

class A implements Serializable {
  var $a = 123;
  function serialize() {
 return serialize($this->a);
 }
  function unserialize($s) {
 $this->a = unserialize($s);
 }
}

<<__EntryPoint>>
function main_1819() {
$o = new A;
apc_store('key', $o);
$r = __hhvm_intrinsics\apc_fetch_no_check('key');
var_dump($r);
}

<?hh

class A implements Serializable {
  public $a = 123;
  function serialize() {
    return serialize($this->a);
  }
  function unserialize($s) {
    $this->a = unserialize($s);
  }
}

<<__EntryPoint>>
function main() {
  $o = new A;
  apc_store('key', $o);
  $r = __hhvm_intrinsics\apc_fetch_no_check('key');
  var_dump($r);
}

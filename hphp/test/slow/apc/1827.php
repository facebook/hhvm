<?hh

class X implements Serializable {
  public function serialize() {
    return 'true';
  }
  public function unserialize($serialized ) {
  }
}
function test() {
  $a = varray[];
  $a[] = $x = new X;
  $a[] = $x;
  $a[] = $x;
  apc_store('foo', $a);
  $a = __hhvm_intrinsics\apc_fetch_no_check('foo');
  var_dump($a);
  $a = __hhvm_intrinsics\apc_fetch_no_check('foo');
  var_dump($a);
}

<<__EntryPoint>>
function main_1827() {
test();
}

<?hh

class X implements Serializable {
  public function serialize() :mixed{
    return 'true';
  }
  public function unserialize($serialized ) :mixed{
  }
}
function test() :mixed{
  $a = vec[];
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
function main_1827() :mixed{
test();
}

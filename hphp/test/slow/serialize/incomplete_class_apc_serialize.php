<?hh

<<__EntryPoint>>
function main() :mixed{
  $str = 'O:1:"X":0:{}';
  $obj = unserialize($str);
  var_dump($obj); // incomplete class
  apc_store('foo', $obj);
  include 'incomplete_class_apc_serialize.inc';
  $o2 = __hhvm_intrinsics\apc_fetch_no_check('foo');
  var_dump($o2); // real X
}

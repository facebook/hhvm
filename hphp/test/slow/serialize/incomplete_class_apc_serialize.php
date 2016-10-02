<?php
function main() {
  $str = 'O:1:"X":0:{}';
  $obj = unserialize($str);
  var_dump($obj); // incomplete class
  apc_store('foo', $obj);
  class X {};
  $o2 = apc_fetch('foo');
  var_dump($o2); // real X
}
main();

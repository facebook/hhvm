<?php

class X implements Serializable {
  public function serialize() {
    return 'true';
  }
  public function unserialize($serialized ) {
  }
}
function test() {
  $a[] = $x = new X;
  $a[] = $x;
  $a[] = $x;
  apc_store('foo', $a);
  $a = apc_fetch('foo');
  var_dump($a);
  $a = apc_fetch('foo');
  var_dump($a);
}
test();

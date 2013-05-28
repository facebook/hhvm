<?php

class A implements Serializable {
  var $a = 123;
  function serialize() {
 return serialize($this->a);
 }
  function unserialize($s) {
 $this->a = unserialize($s);
 }
}
$o = new A;
apc_store('key', $o);
$r = apc_fetch('key');
var_dump($r);

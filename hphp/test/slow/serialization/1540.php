<?php

class A implements Serializable {
  public $__foo = true;
  public function serialize() {
    return serialize(array('a' => 'apple', 'b' => 'banana'));
  }
  public function unserialize($serialized) {
    $props = unserialize($serialized);
    $this->a = $props['a'];
    $this->b = $props['b'];
  }
}
 $obj = unserialize(serialize(new A()));
 var_dump($obj->b);

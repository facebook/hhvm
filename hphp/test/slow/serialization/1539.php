<?php

class A implements Serializable {
  public $__foo = true;
  public function serialize() {
    return null;
  }
  public function unserialize($serialized) {
  }
}
 var_dump(unserialize(serialize(new A())));

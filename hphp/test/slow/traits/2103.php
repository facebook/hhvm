<?php

trait TraitFoo {
  public function testDoSomethingInTrait() {
    return call_user_func(array($this, 'doSomethingInTrait'));
  }
  public function testDoSomethingPublicInTrait() {
    return call_user_func(array($this, 'doSomethingPublicInTrait'));
  }
}
class A {
  use TraitFoo;
  public function testDoSomething() {
    return call_user_func(array($this, 'doSomething'));
  }
  public function __call($name, $args) {
    echo "**calling __call $name**";
  }
  protected function doSomething() {
    return 'doSomething';
  }
  protected function doSomethingInTrait() {
    return 'doSomethingInTrait';
  }
  public function doSomethingPublicInTrait() {
    return 'doSomethingPublicInTrait';
  }
}
$a = new A();
echo $a->testDoSomething()."
";
echo $a->testDoSomethingInTrait()."
";
echo $a->testDoSomethingPublicInTrait()."
";

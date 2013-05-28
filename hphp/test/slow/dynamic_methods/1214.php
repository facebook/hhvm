<?php

abstract class Parent_ {
  protected function overridden() {
    var_dump(isset($this), __METHOD__);
  }
  protected function calledHere() {
    if (isset($this)) {
      $this->overridden();
    }
 else {
      static::overridden();
    }
  }
}
class Child extends Parent_ {
  protected final function overridden() {
    var_dump(isset($this), __METHOD__);
  }
  protected function calledHere() {
    var_dump(__METHOD__);
  }
  public function entry($fun = 'calledHere') {
    self::callParent($fun);
  }
  function callParent($fun) {
    parent::$fun();
    if (isset($this)) {
      $this::$fun();
    }
  }
}
$c = new Child;
$c->entry();
$c->entry('overridden');
Child::entry();
Child::callParent('calledHere');
Child::callParent('overridden');

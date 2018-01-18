<?php

class Base  {
  public function overriddenMethod() {
    return "Base::overriddenMethod";
  }
  public function wrapper() {
    return $this->overriddenMethod();
  }
}

class DerivedOne extends Base {
  public function overriddenMethod() {
    return "DerivedOne::overriddenMethod";
  }
}

class DerivedTwo extends Base {
}

class_alias( 'Base', 'AliasForBase' );

$inst = new DerivedOne();
printf($inst->wrapper());

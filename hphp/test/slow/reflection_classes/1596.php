<?php

class Cloneable {}

class NotCloneable_privateConstructor {
  private function __construct() {

  }
}

interface NotCloneable_interface {}

trait NotCloneable_trait {}

class NotCloneable_protectedClone {
  protected function __clone() {

  }
}

class NotCloneable_privateClone {
  private function __clone() {

  }
}

abstract class NotCloneable_abstractClone {
  abstract function __clone();
}

function isCloneable($class_name) {
  $info = new ReflectionClass($class_name);
  return $info->isCloneable();
}

var_dump(isCloneable('Cloneable'));
var_dump(isCloneable('NotCloneable_interface'));
var_dump(isCloneable('NotCloneable_trait'));
var_dump(isCloneable('NotCloneable_protectedClone'));
var_dump(isCloneable('NotCloneable_privateClone'));
var_dump(isCloneable('NotCloneable_abstractClone'));

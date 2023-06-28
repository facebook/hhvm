<?hh

class Cloneable {}

class NotCloneable_privateConstructor {
  private function __construct() {

  }
}

interface NotCloneable_interface {}

trait NotCloneable_trait {}

class NotCloneable_protectedClone {
  protected function __clone() :mixed{

  }
}

class NotCloneable_privateClone {
  private function __clone() :mixed{

  }
}

abstract class NotCloneable_abstractClone {
  abstract function __clone():mixed;
}

function isCloneable($class_name) :mixed{
  $info = new ReflectionClass($class_name);
  return $info->isCloneable();
}


<<__EntryPoint>>
function main_1596() :mixed{
var_dump(isCloneable('Cloneable'));
var_dump(isCloneable('NotCloneable_interface'));
var_dump(isCloneable('NotCloneable_trait'));
var_dump(isCloneable('NotCloneable_protectedClone'));
var_dump(isCloneable('NotCloneable_privateClone'));
var_dump(isCloneable('NotCloneable_abstractClone'));
}

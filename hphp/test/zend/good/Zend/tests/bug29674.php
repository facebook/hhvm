<?hh

class BaseClass {
  private $private_base = "Base";

  function printVars() :mixed{
    var_dump($this->private_base);
    try {
      var_dump($this->private_child);
    } catch (UndefinedPropertyException $e) {
      var_dump($e->getMessage());
    }
  }
}

class ChildClass extends BaseClass {
  private $private_child = "Child";
}
<<__EntryPoint>>
function main(): void {
  echo "===BASE===\n";
  $obj = new BaseClass;
  $obj->printVars();

  echo "===CHILD===\n";
  $obj = new ChildClass;
  $obj->printVars();

  echo "===DONE===\n";
}

<?hh
class NewStyle {
  public function __construct() {
  }
}

class SubNewStyle extends NewStyle {
}

<<__EntryPoint>>
function main_class_constructors() :mixed{
var_dump((new ReflectionClass('NewStyle'))->getConstructor()->getName());
var_dump((new ReflectionClass('SubNewStyle'))->getConstructor()->getName());
}

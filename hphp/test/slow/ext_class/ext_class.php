<?hh
class test implements itestable {
  public $foo;
  protected $prop;
  private $bar;
  public function foo() :mixed{}
  protected function func() :mixed{}
  function bar() :mixed{}
  const const_foo = 'f';
}

interface itestable {
  function foo():mixed;
  function bar():mixed;
}


<<__EntryPoint>>
function main_ext_class() :mixed{
$classes = get_declared_classes();
var_dump(in_array("test", $classes, true));

$interfaces = get_declared_interfaces();
var_dump(in_array("itestable", $interfaces, true));

var_dump(class_exists("TEst"));

var_dump(interface_exists("iTESTable"));

var_dump(get_class_methods("TEst")[0] === "foo");

var_dump(get_class_vars("TEst") === dict["foo" => null]);

var_dump(get_class_constants("test") === dict["const_foo" => "f"]);
}

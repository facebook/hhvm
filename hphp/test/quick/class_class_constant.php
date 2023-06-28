<?hh

class Class1 {
}

class Class2 {
  <<Attribute(Class1::class)>>  // Should allow builtin class class constant
    public function index() :mixed{}
}

function get_attributes(): void {
  $rc = new ReflectionClass("Class2");
  var_dump($rc->getMethods()[0]->getAttributes()["Attribute"]);
}
<<__EntryPoint>> function main(): void {
echo get_attributes();
}

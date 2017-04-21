<?hh

class Class1 {
  const member1 = "member1";
}

class Class2 {
  // Not allowed.  Can't use user defined
  // class constants in attribute
  <<Attribute(Class1::member1)>>
    public function index() {}
}

function get_attributes(): void {
  $rc = new ReflectionClass("Class2");
  var_dump($rc->getMethods()[0]->getAttributes()["Attribute"]);
}

echo get_attributes();

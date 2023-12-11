<?hh

class noCtor {
}

class publicCtorNew {
    public function __construct() {}
}

class protectedCtorNew {
    protected function __construct() {}
}

class privateCtorNew {
    private function __construct() {}
}

<<__EntryPoint>>
function main() :mixed{
  $classes = vec["noCtor", "publicCtorNew", "protectedCtorNew", "privateCtorNew"];

  foreach($classes  as $class ) {
    $reflectionClass = new ReflectionClass($class);
    echo "Is $class instantiable?  ";
    var_dump($reflectionClass->isInstantiable());
  }
}

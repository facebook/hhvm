<?hh

class noCtor {
  public static function reflectionObjectFactory() :mixed{
    return new ReflectionObject(new self);
  }
}

class publicCtorNew {
  public function __construct() {}
  public static function reflectionObjectFactory() :mixed{
    return new ReflectionObject(new self);
  }
}

class protectedCtorNew {
  protected function __construct() {}
  public static function reflectionObjectFactory() :mixed{
    return new ReflectionObject(new self);
  }
}

class privateCtorNew {
  private function __construct() {}
  public static function reflectionObjectFactory() :mixed{
    return new ReflectionObject(new self);
  }
}

<<__EntryPoint>>
function main() :mixed{
  $reflectionObjects = vec[
    noCtor::reflectionObjectFactory(),
    publicCtorNew::reflectionObjectFactory(),
    protectedCtorNew::reflectionObjectFactory(),
    privateCtorNew::reflectionObjectFactory(),
  ];

  foreach ($reflectionObjects as $reflectionObject) {
    $name = $reflectionObject->getName();
    echo "Is $name instantiable? ";
    var_dump($reflectionObject->isInstantiable());
  }
}

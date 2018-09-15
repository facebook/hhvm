<?hh

class X extends ReflectionProperty {
  function __construct() {}
}

$x = new X;
var_dump($x->isPublic());

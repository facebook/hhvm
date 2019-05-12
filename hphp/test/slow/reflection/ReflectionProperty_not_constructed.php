<?hh

class X extends ReflectionProperty {
  function __construct() {}
}
<<__EntryPoint>> function main(): void {
$x = new X;
var_dump($x->isPublic());
}

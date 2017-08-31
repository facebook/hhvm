<?hh

type MyType = void;

$x = new ReflectionTypeAlias('mytype');
echo $x->getName(), "\n";

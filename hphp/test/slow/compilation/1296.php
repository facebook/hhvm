<?hh

function test($className) :mixed{
$x = new ReflectionClass($className);
return $x->newInstance()->loadAll();
 }
<<__EntryPoint>> function main(): void { echo "Done.\n"; }

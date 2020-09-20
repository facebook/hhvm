<?hh

function test($className) {
$x = new ReflectionClass($className);
return $x->newInstance()->loadAll();
 }
<<__EntryPoint>> function main(): void { echo "Done.\n"; }

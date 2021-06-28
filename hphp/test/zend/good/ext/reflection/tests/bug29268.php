<?hh

class A {}

class B{
    public function doit(A $a){
    }
}
<<__EntryPoint>> function main(): void {
$ref = new ReflectionMethod('B','doit');
$parameters = $ref->getParameters();
foreach($parameters as $parameter)
{
    $class = $parameter->getClass();
    echo $class->name."\n";
}
echo "ok\n";
}

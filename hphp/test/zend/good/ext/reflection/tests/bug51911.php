<?hh

class Foo {
   const X = 1;
   public function x($x = varray[1]) {}
}
<<__EntryPoint>> function main(): void {
$clazz = new ReflectionClass('Foo');
$method = $clazz->getMethod('x');
foreach ($method->getParameters() as $param) {
    if ( $param->isDefaultValueAvailable())
        echo '$', $param->getName(), ' : ', var_export($param->getDefaultValue(), true), "\n";
}
}

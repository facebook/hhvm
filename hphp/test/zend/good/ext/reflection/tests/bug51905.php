<?hh

class Bar {
    const Y = 20;
}

class Foo extends Bar {
    const X = 12;
    public function x($x = 1, $y = vec[self::X], $z = parent::Y) :mixed{}
}
<<__EntryPoint>> function main(): void {
$clazz = new ReflectionClass('Foo');
$method = $clazz->getMethod('x');
foreach ($method->getParameters() as $param) {
    if ( $param->isDefaultValueAvailable())
        echo '$', $param->getName(), ' : ', var_export($param->getDefaultValue(), true), "\n";
}
}

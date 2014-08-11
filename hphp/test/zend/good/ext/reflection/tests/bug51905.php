<?php

class Bar {
	const Y = 20;
}

class Foo extends Bar {
	const X = 12;
	public function x($x = 1, $y = array(self::X), $z = parent::Y) {}
}

$clazz = new ReflectionClass('Foo');
$method = $clazz->getMethod('x');
foreach ($method->getParameters() as $param) {
    if ( $param->isDefaultValueAvailable())
        echo '$', $param->getName(), ' : ', var_export($param->getDefaultValue(), 1), "\n";
}

?>

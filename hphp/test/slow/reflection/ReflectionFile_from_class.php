<?hh // strict

class ExampleClass {}

$class = new ReflectionClass('ExampleClass');
$fileViaClass = $class->getFile();
var_dump($fileViaClass->getName() === __FILE__);
var_dump($fileViaClass->getAttributesNamespaced());

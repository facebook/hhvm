<?hh

<<file: MyFileAttribute>>

class MyFileAttribute implements \HH\FileAttribute {}

class ExampleClass {}
<<__EntryPoint>> function main(): void {
$class = new ReflectionClass('ExampleClass');
$fileViaClass = $class->getFile();
var_dump($fileViaClass->getName() === __FILE__);
var_dump($fileViaClass->getAttributesNamespaced());
}

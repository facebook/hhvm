<?hh
<<__EntryPoint>> function main(): void {
$file = new ReflectionFile(__FILE__);
var_dump($file->getName() === __FILE__);
var_dump($file->getAttributesNamespaced());
}

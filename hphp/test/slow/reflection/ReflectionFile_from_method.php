<?hh

class ExampleClass {
  public static function exampleMethod($arg) :mixed{
    echo "called exampleMethod\n";
  }
}
<<__EntryPoint>> function main(): void {
$methodV1 = new ReflectionClass('ExampleClass')->getMethod('exampleMethod');
$fileViaMethodV1 = $methodV1->getFile();
var_dump($fileViaMethodV1->getName() === __FILE__);
var_dump($fileViaMethodV1->getAttributesNamespaced());


$methodV2 = new ReflectionMethod('ExampleClass', 'exampleMethod');
$fileViaMethodV2 = $methodV2->getFile();
var_dump($fileViaMethodV2->getName() === __FILE__);
var_dump($fileViaMethodV2->getAttributesNamespaced());
}

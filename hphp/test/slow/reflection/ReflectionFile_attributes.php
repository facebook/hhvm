<?hh // strict

<<file: MyFileAttribute>>

class MyFileAttribute implements \HH\FileAttribute {}

$file =
  new ReflectionFile('hphp/test/slow/reflection/ReflectionFile_attributes.php');
var_dump($file->getName() === __FILE__);
var_dump($file->getAttributesNamespaced());

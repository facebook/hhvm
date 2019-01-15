<?hh // strict

<<file: MyFileAttribute>>

class MyFileAttribute implements \HH\FileAttribute {}

$file = new ReflectionFile(__FILE__);
var_dump($file->getName() === __FILE__);
var_dump($file->getAttributesNamespaced());

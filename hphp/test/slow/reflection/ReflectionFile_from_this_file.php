<?hh // strict

$file = new ReflectionFile(
  'hphp/test/slow/reflection/ReflectionFile_from_this_file.php',
);
var_dump($file->getName() === __FILE__);
var_dump($file->getAttributesNamespaced());

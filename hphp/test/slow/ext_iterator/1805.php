<?hh


<<__EntryPoint>>
function main_1805() :mixed{
try {
  $y = new RecursiveDirectoryIterator('/fake_path');
  $z = new RecursiveIteratorIterator($y);
  $z->next();
}
 catch (UnexpectedValueException $e) {
}
var_dump('ok');
}

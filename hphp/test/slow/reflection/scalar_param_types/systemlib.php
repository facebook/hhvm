<?hh


<<__EntryPoint>>
function main_systemlib() :mixed{
$rc = new ReflectionMethod('FilesystemIterator', 'setFlags');
$flags = $rc->getParameters()[0];
var_dump($flags->getClass());
var_dump($flags->getTypehintText());
}

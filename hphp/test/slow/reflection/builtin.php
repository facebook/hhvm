<?hh


<<__EntryPoint>>
function main_builtin() :mixed{
$f = new ReflectionFunction("json_encode");
var_dump($f->getStartLine(), $f->getEndLine(), $f->getFileName());

$c = new ReflectionClass("DateTime");
var_dump($c->getStartLine(), $c->getEndLine(), $f->getFileName());
}

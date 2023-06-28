<?hh

class C {
public function me() :mixed{
echo 'fail';
}
}

<<__EntryPoint>>
function main_1359() :mixed{
$ref = new ReflectionClass('C');
var_dump($ref->hasMethod('me'));
var_dump($ref->hasMethod('me'));
$m = $ref->getMethod('me');
var_dump($m->getName());
}

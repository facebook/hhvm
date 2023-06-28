<?hh


<<__EntryPoint>>
function main_803() :mixed{
echo '=== Vector ===', "\n";
$v = Vector {};
$v[] = 123;

var_dump($v->toVArray());
var_dump($v->pop());
var_dump($v->toVArray());

echo '=== Set ===', "\n";
$s = Set {};
var_dump($s->toDArray());
$s[] = 123;
$s[] = '123';
$s[] = 'foo';

$arr = $s->toDArray();
var_dump($arr);
var_dump((count($s)) === count($arr));
$s->remove('123');
var_dump($s->toDArray()); // on collision, last entry wins
}

<?hh

echo '=== Vector ===', "\n";
$v = Vector {};
$v[] = 123;

var_dump($v->toArray());
var_dump($v->pop());
var_dump($v->toArray());

echo '=== Set ===', "\n";
$s = Set {};
var_dump($s->toArray());
$s[] = 123;
$s[] = '123';
$s[] = 'foo';

$arr = $s->toArray();
var_dump($arr);
var_dump((count($s) - 1) === count($arr));
$s->remove('123');
var_dump($s->toArray()); // on collision, last entry wins

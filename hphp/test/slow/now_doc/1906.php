<?hh


<<__EntryPoint>>
function main_1906() :mixed{
$b = 'bad';
$a = <<<'NOWDOC'
$b
NOWDOC;
var_dump($a);
$a = <<<"NOWDOC"
$b
NOWDOC;
var_dump($a);
$a = <<<NOWDOC
$b
NOWDOC;
var_dump($a);
}

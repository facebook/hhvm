<?hh


<<__EntryPoint>>
function main_1907() :mixed{
$a = <<<NOWDOC
"'\t
NOWDOC;
var_dump($a);
$a = <<<'NOWDOC'
"'\t
NOWDOC;
var_dump($a);
$a = <<<"NOWDOC"
"'\t
NOWDOC;
var_dump($a);
}

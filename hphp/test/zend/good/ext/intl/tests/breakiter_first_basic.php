<?hh <<__EntryPoint>> function main(): void {
ini_set("intl.error_level", E_WARNING);

$bi = IntlBreakIterator::createWordInstance('pt');
$bi->setText('foo bar trans');

var_dump($bi->current());
var_dump($bi->next());
var_dump($bi->first());
var_dump($bi->current());
}

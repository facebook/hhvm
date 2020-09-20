<?hh <<__EntryPoint>> function main(): void {
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "pt_PT");

$bi = IntlBreakIterator::createWordInstance('pt');
var_dump($bi->current());
$bi->setText('foo bar trans zoo bee');

var_dump($bi->first());
var_dump($bi->current());
var_dump($bi->next());
var_dump($bi->current());
echo "==DONE==";
}

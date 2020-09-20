<?hh <<__EntryPoint>> function main(): void {
ini_set("intl.error_level", E_WARNING);

$bi = IntlBreakIterator::createWordInstance('pt');
var_dump($bi->setText('foo bar'));
var_dump($bi->getText());
var_dump($bi->setText('1'));
var_dump($bi->getText());
var_dump($bi->setText('aaa'));
var_dump($bi->getText());

/* setText resets the pointer */
var_dump($bi->next());
var_dump($bi->setText('foo bar'));
var_dump($bi->current());
}

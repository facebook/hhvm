<?hh <<__EntryPoint>> function main(): void {
ini_set("intl.error_level", E_WARNING);

$bi = new IntlRuleBasedBreakIterator('[\p{Letter}\uFFFD]+;[:number:]+;');
$bi_clone = clone $bi;
var_dump(get_class($bi), get_class($bi_clone));
var_dump($bi == $bi_clone);

$bi->setText('foobar');
$bi_clone = clone $bi;
var_dump(get_class($bi), get_class($bi_clone));
var_dump($bi == $bi_clone);
}

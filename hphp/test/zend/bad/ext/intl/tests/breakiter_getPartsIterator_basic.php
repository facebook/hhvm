<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "pt_PT");

$bi = IntlBreakIterator::createWordInstance('pt');
$pi = $bi->getPartsIterator();
var_dump(get_class($pi));
print_r(iterator_to_array($pi));

$bi->setText("foo bar");
$pi = $bi->getPartsIterator();
var_dump(get_class($pi->getBreakIterator()));
print_r(iterator_to_array($pi));
var_dump($pi->getRuleStatus());
?>
==DONE==
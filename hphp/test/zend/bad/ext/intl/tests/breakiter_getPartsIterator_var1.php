<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "pt_PT");

$text = 'foo bar tao';

$it = IntlBreakIterator::createWordInstance(NULL);
$it->setText($text);

var_dump(iterator_to_array($it->getPartsIterator(IntlPartsIterator::KEY_SEQUENTIAL)));
var_dump(iterator_to_array($it->getPartsIterator(IntlPartsIterator::KEY_LEFT)));
var_dump(iterator_to_array($it->getPartsIterator(IntlPartsIterator::KEY_RIGHT)));

?>
==DONE==
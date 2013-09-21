<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "pt_PT");

$it = IntlBreakIterator::createWordInstance(NULL);
var_dump($it->getPartsIterator(array()));
var_dump($it->getPartsIterator(1, 2));
var_dump($it->getPartsIterator(-1));

?>
==DONE==
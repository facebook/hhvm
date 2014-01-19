<?php
ini_set("intl.error_level", E_WARNING);

$it = IntlBreakIterator::createCodePointInstance();
var_dump($it->getLastCodePoint(array()));
<?php
ini_set("intl.error_level", E_WARNING);

var_dump(IntlBreakIterator::createWordInstance(array()));
var_dump(IntlBreakIterator::createSentenceInstance(NULL, 2));
var_dump(IntlBreakIterator::createCharacterInstance(NULL, 2));
var_dump(IntlBreakIterator::createTitleInstance(NULL, 2));
var_dump(IntlBreakIterator::createLineInstance(NULL, 2));


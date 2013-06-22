<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "pt_PT");

$text = 'ตัวอย่างข้อความ';

$it = IntlBreakIterator::createCodePointInstance()->getPartsIterator();
$it->getBreakIterator()->setText($text);

foreach ($it as $k => $v) {
	echo "$k. $v (" . sprintf("U+%04X", $it->getBreakIterator()->getLastCodePoint()) .
		") at {$it->getBreakIterator()->current()}\r\n";
}

?>
==DONE==
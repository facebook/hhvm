<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "pt_PT");

$text = 'ตัวอย่างข้อความ';

$codepoint_it = IntlBreakIterator::createCodePointInstance();
var_dump(get_class($codepoint_it));
$codepoint_it->setText($text);

print_r(iterator_to_array($codepoint_it));

?>
==DONE==
<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "ja");

$m = array('createWordInstance', 'createLineInstance', 'createCharacterInstance',
	'createSentenceInstance', 'createTitleInstance');

$t = 'Frase 1... Frase 2'.

$o1 = $o2 = null;
foreach ($m as $method) {
	echo "===== $method =====\n";
	$o1 = call_user_func(array('IntlBreakIterator', $method), 'ja');
	var_dump($o1 == $o2);
	$o2 = call_user_func(array('IntlBreakIterator', $method), NULL);
	var_dump($o1 == $o2);
	echo "\n";
}
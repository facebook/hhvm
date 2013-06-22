<?php
ini_set("intl.use_exceptions", true);
$t = transliterator_create('any-hex');
try {
	var_dump($t->transliterate('a', 3));
} catch (IntlException $intlE) {
	var_dump($intlE->getMessage());
}
ini_set("intl.use_exceptions", false);
ini_set("intl.error_level", E_NOTICE);
var_dump($t->transliterate('a', 3));
<?php
ini_set("intl.error_level", E_WARNING);
$t = Transliterator::create("[\p{Bidi_Mirrored}] Hex");
var_dump($t->transliterate("\x8F"));
echo transliterator_get_error_code($t), "\n";

echo $t->getErrorCode(), "\n";

var_dump($t->transliterate(""));
echo $t->getErrorCode(), "\n";

echo "Done.\n";
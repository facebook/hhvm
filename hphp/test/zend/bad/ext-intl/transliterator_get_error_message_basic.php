<?php
ini_set("intl.error_level", E_WARNING);
$t = Transliterator::create("[\p{Bidi_Mirrored}] Hex");
var_dump($t->transliterate("\x8F"));
echo transliterator_get_error_message($t), "\n";

echo $t->getErrorMessage(), "\n";

var_dump($t->transliterate(""));
echo $t->getErrorMessage(), "\n";

echo "Done.\n";
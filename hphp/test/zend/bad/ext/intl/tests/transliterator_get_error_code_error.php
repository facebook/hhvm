<?php
ini_set("intl.error_level", E_WARNING);
$t = Transliterator::create("[\p{Bidi_Mirrored}] Hex");
echo transliterator_get_error_code(), "\n";
echo $t->getErrorCode(null), "\n";
echo transliterator_get_error_code(array()), "\n";

<?php
ini_set("intl.error_level", E_WARNING);
//exec('pause');
$str = " o";
echo transliterator_transliterate("[\p{White_Space}] hex", $str), "\n";

echo transliterator_transliterate("\x8F", $str), "\n";
echo intl_get_error_message(), "\n";

class A {
function __toString() { return "inexistent id"; }
}

echo transliterator_transliterate(new A(), $str), "\n";
echo intl_get_error_message(), "\n";

echo "Done.\n";
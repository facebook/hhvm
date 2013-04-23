<?php

ini_set("intl.error_level", E_WARNING);

$tr = Transliterator::create("latin");

//Arguments
var_dump(transliterator_transliterate());
var_dump(transliterator_transliterate($tr,array()));
var_dump(transliterator_transliterate($tr,"str",7));
var_dump(transliterator_transliterate($tr,"str",7,6));
var_dump(transliterator_transliterate($tr,"str",2,-1,"extra"));

//Arguments
var_dump($tr->transliterate());
var_dump($tr->transliterate(array()));

//bad UTF-8
transliterator_transliterate($tr, "\x80\x03");

echo "Done.\n";
<?php

ini_set("intl.error_level", E_WARNING);

$tr = Transliterator::create("Katakana-Latin");
$tr->createInverse(array());

$tr = Transliterator::create("Katakana-Latin");
transliterator_create_inverse("jj");

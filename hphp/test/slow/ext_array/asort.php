<?php

$fruits = array(
  "d" => "lemon",
  "a" => "orange",
  "b" => "banana",
  "c" => "apple"
);

asort($fruits);
var_dump($fruits);

$arr = array("at", "\xe0s", "as");
i18n_loc_set_default("en_US");
asort($arr, 0, true);
$arr = array("num2ber", "num1ber", "num10ber");
i18n_loc_set_default("en_US");
i18n_loc_set_attribute(UCOL_NUMERIC_COLLATION, UCOL_ON);
i18n_loc_set_strength(UCOL_PRIMARY);
asort($arr, SORT_REGULAR, true);
i18n_loc_set_attribute(UCOL_NUMERIC_COLLATION, UCOL_DEFAULT);
i18n_loc_set_strength(UCOL_DEFAULT);
var_dump($arr);

$arr = array("G\xediron",        // &iacute; (Latin-1)
                     "G\xc3\xb3nzales",  // &oacute; (UTF-8)
                     "G\xc3\xa9 ara",    // &eacute; (UTF-8)
                     "G\xe1rcia");       // &aacute; (Latin-1)
i18n_loc_set_default("en_US");
i18n_loc_set_attribute(UCOL_NUMERIC_COLLATION, UCOL_ON);
i18n_loc_set_strength(UCOL_PRIMARY);
asort($arr, SORT_REGULAR, true);
i18n_loc_set_attribute(UCOL_NUMERIC_COLLATION, UCOL_DEFAULT);
i18n_loc_set_strength(UCOL_DEFAULT);
var_dump($arr);

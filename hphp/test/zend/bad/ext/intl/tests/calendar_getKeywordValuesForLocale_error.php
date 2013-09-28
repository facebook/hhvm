<?php
ini_set("intl.error_level", E_WARNING);

var_dump(intlcal_get_keyword_values_for_locale(1, 2));
var_dump(IntlCalendar::getKeywordValuesForLocale(1, 2, array()));

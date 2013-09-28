<?php
ini_set("intl.error_level", E_WARNING);

var_dump(intlcal_get_available_locales(1));
var_dump(IntlCalendar::getAvailableLocales(2));

<?php
ini_set("intl.error_level", E_WARNING);

var_dump(IntlTimeZone::createEnumeration(array()));
var_dump(IntlTimeZone::createEnumeration(1, 2));


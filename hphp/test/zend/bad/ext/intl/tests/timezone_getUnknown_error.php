<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');

IntlTimeZone::getUnknown(1);

intltz_get_unknown(1);

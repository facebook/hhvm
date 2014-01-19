<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');

var_dump($c->getErrorMessage(array()));

var_dump(intlcal_get_error_message(null));

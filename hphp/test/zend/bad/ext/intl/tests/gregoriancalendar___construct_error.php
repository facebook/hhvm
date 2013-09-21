<?php
ini_set("intl.error_level", E_WARNING);

var_dump(intlgregcal_create_instance(1,2,3,4,5,6,7));
var_dump(intlgregcal_create_instance(1,2,3,4,5,6,7,8));
var_dump(intlgregcal_create_instance(1,2,3,4));
var_dump(new IntlGregorianCalendar(1,2,NULL,4));
var_dump(new IntlGregorianCalendar(1,2,3,4,NULL,array()));


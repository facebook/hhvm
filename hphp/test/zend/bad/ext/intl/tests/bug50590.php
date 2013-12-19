<?php

$fmt = new IntlDateFormatter("en_US", IntlDateFormatter::FULL, IntlDateFormatter::FULL);
var_dump($fmt->parse("Wednesday, January 20, 2038 3:14:07 AM GMT"));

?>
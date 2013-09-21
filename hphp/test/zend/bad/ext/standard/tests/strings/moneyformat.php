<?php
setlocale(LC_MONETARY, 'en_US');
var_dump( money_format("X%nY", 3.1415));
var_dump(money_format("AAAAA%n%n%n%n", NULL));
?>
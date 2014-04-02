<?php
$kilo = str_repeat("\xf4", 1000);
echo strlen(quoted_printable_encode(str_repeat($kilo, 1000000)));

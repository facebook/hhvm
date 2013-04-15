<?php

 $a = array("a bc", "\xc1 bc", "d ef");asort($a);print_r($a);$a = array("a bc", "\xc1 bc", "d ef");asort($a, SORT_LOCALE_STRING);print_r($a);$a = array("a bc", "\xc1 bc", "d ef");setlocale(LC_ALL, "pt_PT");asort($a);print_r($a);$a = array("a bc", "\xc1 bc", "d ef");setlocale(LC_ALL, "pt_PT");asort($a, SORT_LOCALE_STRING);print_r($a);
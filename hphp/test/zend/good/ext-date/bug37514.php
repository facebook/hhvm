<?php
date_default_timezone_set('UTC');
echo date('r', strtotime('May 18th 5:05', 1168156376)), "\n";
echo date('r', strtotime('May 18th 5:05pm', 1168156376)), "\n";
echo date('r', strtotime('May 18th 5:05 pm', 1168156376)), "\n";
echo date('r', strtotime('May 18th 5:05am', 1168156376)), "\n";
echo date('r', strtotime('May 18th 5:05 am', 1168156376)), "\n";
echo date('r', strtotime('May 18th 2006 5:05pm', 1168156376)), "\n";
?>
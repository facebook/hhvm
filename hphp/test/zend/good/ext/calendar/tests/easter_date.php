<?php
putenv('TZ=UTC');
echo date("Y-m-d", easter_date(2000))."\n";       
echo date("Y-m-d", easter_date(2001))."\n";      
echo date("Y-m-d", easter_date(2002))."\n";      
echo date("Y-m-d", easter_date(1492))."\n";
?>
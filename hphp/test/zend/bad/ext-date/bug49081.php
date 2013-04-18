<?php
   date_default_timezone_set('Europe/Berlin');
   $d1 = new DateTime('2010-01-01 06:00:00');
   $d2 = new DateTime('2010-01-31 10:00:00');
   $d  = $d1->diff($d2);
   print_r($d);
?>
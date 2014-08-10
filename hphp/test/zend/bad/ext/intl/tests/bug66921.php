<?php
$f = new ReflectionFunction('intltz_from_date_time_zone');
var_dump($f->getParameters()[0]->getClass());

?>

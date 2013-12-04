<?php
date_default_timezone_set('Europe/Prague');
var_dump(unserialize('O:8:"DateTime":0:{}') == new DateTime);
?>
<?php

//bogus
var_dump(date_default_timezone_set('AAA'));
var_dump(date_default_timezone_set('ZZZ'));


//now the first and the last one
var_dump(date_default_timezone_set("Africa/Abidjan"));
var_dump(date_default_timezone_set("Zulu"));

echo "done\n";

?>